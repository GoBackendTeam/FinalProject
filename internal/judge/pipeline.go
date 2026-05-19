package judge

import (
	"context"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/archive"
	"github.com/GoBackendTeam/FinalProject/internal/config"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/GoBackendTeam/FinalProject/internal/store"
)

// pipeline 執行單一提交的完整評測管線。
type pipeline struct {
	cfg    *config.Config
	st     *store.Store
	docker *dockerRunner
}

// paths 為某次提交在 app 視角 / 主機視角下的工作目錄。
type paths struct {
	appBase   string // app 看到的 <workspace>/<id>
	hostBase  string // 主機看到的 <workspace_host>/<id>(bind mount 用)
}

func (p paths) appDir(sub string) string  { return filepath.Join(p.appBase, sub) }
func (p paths) hostDir(sub string) string { return filepath.Join(p.hostBase, sub) }

func (pl *pipeline) workspacePaths(id string) paths {
	return paths{
		appBase:  filepath.Join(pl.cfg.JudgeWorkspace, id),
		hostBase: filepath.Join(pl.cfg.JudgeWorkspaceHost, id),
	}
}

// Run 跑完整管線並把結果寫回 DB。回傳的 error 僅代表系統內部錯誤(IE)。
func (pl *pipeline) Run(ctx context.Context, sub *model.Submission, prob *model.Problem) error {
	now := time.Now()
	sub.StartedAt = &now
	sub.Status = model.StatusRunning
	pl.st.DB.Save(sub)

	ps := pl.workspacePaths(sub.ID)
	for _, d := range []string{"src", "build", "logs"} {
		if err := os.MkdirAll(ps.appDir(d), 0o755); err != nil {
			return pl.fail(sub, model.StatusIE, "mkdir workspace: "+err.Error())
		}
	}

	sub.WorkspaceDir = ps.appBase
	sub.ConfigureLogPath = filepath.Join(ps.appDir("logs"), "configure.log")
	sub.CompileLogPath = filepath.Join(ps.appDir("logs"), "compile.log")
	sub.OutputLogPath = filepath.Join(ps.appDir("logs"), "output.log")

	// 1) 解壓使用者上傳的原始碼到 src/
	if err := archive.Extract(sub.SourceArchivePath, ps.appDir("src")); err != nil {
		return pl.fail(sub, model.StatusSE, "extract archive: "+err.Error())
	}

	bindLogs := ps.hostDir("logs") + ":/logs:rw"
	bindBuildRW := ps.hostDir("build") + ":/build:rw"
	bindBuildRO := ps.hostDir("build") + ":/build:ro"
	bindSrcRO := ps.hostDir("src") + ":/src:ro"
	bindProblemRO := prob.ProblemDir + ":/problem:ro"

	// 2) Configure 階段(失敗 → SE),日誌 → configure.log
	confCmd := []string{"sh", "-c",
		"cmake -S /problem -B /build -G Ninja -D SOURCE_ROOT=/src > /logs/configure.log 2>&1"}
	r, err := pl.docker.run(ctx, runSpec{
		Image:       pl.cfg.JudgeImage,
		Cmd:         confCmd,
		Binds:       []string{bindProblemRO, bindSrcRO, bindBuildRW, bindLogs},
		NetworkMode: pl.cfg.JudgeBuildNetwork,
		Timeout:     pl.cfg.JudgeBuildTimeout,
	})
	if err != nil {
		return pl.fail(sub, model.StatusIE, "configure stage: "+err.Error())
	}
	if r.TimedOut {
		return pl.fail(sub, model.StatusSE, "configure timed out")
	}
	if r.ExitCode != 0 {
		return pl.fail(sub, model.StatusSE, fmt.Sprintf("cmake configure failed (exit %d)", r.ExitCode))
	}

	// 3) Build 階段(失敗 → CE),日誌 → compile.log
	buildCmd := []string{"sh", "-c",
		"cmake --build /build --verbose > /logs/compile.log 2>&1"}
	r, err = pl.docker.run(ctx, runSpec{
		Image:       pl.cfg.JudgeImage,
		Cmd:         buildCmd,
		Binds:       []string{bindProblemRO, bindSrcRO, bindBuildRW, bindLogs},
		NetworkMode: pl.cfg.JudgeBuildNetwork,
		Timeout:     pl.cfg.JudgeBuildTimeout,
	})
	if err != nil {
		return pl.fail(sub, model.StatusIE, "build stage: "+err.Error())
	}
	if r.TimedOut {
		return pl.fail(sub, model.StatusCE, "compile timed out")
	}
	if r.ExitCode != 0 {
		return pl.fail(sub, model.StatusCE, fmt.Sprintf("compilation failed (exit %d)", r.ExitCode))
	}

	// 4) 逐 case 在「斷網」容器內執行,日誌 → output.log
	cases := splitCases(prob.CaseNames)
	if len(cases) == 0 {
		return pl.fail(sub, model.StatusIE, "no test cases defined for problem")
	}

	// 清掉舊的 case 結果(支援重跑)。
	pl.st.DB.Where("submission_id = ?", sub.ID).Delete(&model.CaseResult{})

	var results []model.CaseResult
	for _, c := range cases {
		bin, err := findCaseBinary(ps.appDir("build"), c)
		if err != nil {
			results = append(results, model.CaseResult{
				SubmissionID: sub.ID, CaseName: c, Status: model.StatusCE,
			})
			continue
		}
		runCmd := []string{"sh", "-c", fmt.Sprintf(
			"echo '===== case %s =====' >> /logs/output.log; /build/%s >> /logs/output.log 2>&1", c, bin)}

		start := time.Now()
		er, err := pl.docker.run(ctx, runSpec{
			Image:       pl.cfg.JudgeImage,
			Cmd:         runCmd,
			Binds:       []string{bindBuildRO, bindLogs},
			NetworkMode: "none", // PRD:執行容器必須完全斷網
			Timeout:     pl.cfg.JudgeCaseTimeout,
			MemoryBytes: 512 << 20,
		})
		dur := time.Since(start).Milliseconds()
		if err != nil {
			results = append(results, model.CaseResult{
				SubmissionID: sub.ID, CaseName: c, Status: model.StatusIE, DurationMs: dur,
			})
			continue
		}
		results = append(results, model.CaseResult{
			SubmissionID: sub.ID,
			CaseName:     c,
			Status:       classifyCase(er.ExitCode, er.TimedOut, er.OOMKilled),
			ExitCode:     er.ExitCode,
			DurationMs:   dur,
		})
	}
	pl.st.DB.Create(&results)

	final := aggregate(results)
	fin := time.Now()
	sub.Status = final
	sub.FinishedAt = &fin
	sub.Message = summarize(results)
	pl.st.DB.Save(sub)
	return nil
}

func (pl *pipeline) fail(sub *model.Submission, st model.Status, msg string) error {
	fin := time.Now()
	sub.Status = st
	sub.Message = msg
	sub.FinishedAt = &fin
	pl.st.DB.Save(sub)
	return nil
}

func splitCases(csv string) []string {
	var out []string
	for _, c := range strings.Split(csv, ",") {
		if c = strings.TrimSpace(c); c != "" {
			out = append(out, c)
		}
	}
	return out
}

func summarize(results []model.CaseResult) string {
	counts := map[model.Status]int{}
	for _, r := range results {
		counts[r.Status]++
	}
	var b strings.Builder
	for _, s := range []model.Status{model.StatusAC, model.StatusWA, model.StatusRE, model.StatusTLE} {
		if counts[s] > 0 {
			fmt.Fprintf(&b, "%s=%d ", s, counts[s])
		}
	}
	return strings.TrimSpace(b.String())
}

// findCaseBinary 在 build 目錄中尋找名稱以 "-<case>" 結尾的可執行檔,
// 回傳相對於 build 目錄的路徑(對應容器內 /build/<rel>)。
func findCaseBinary(buildDir, caseName string) (string, error) {
	var found string
	suffix := "-" + caseName
	err := filepath.WalkDir(buildDir, func(path string, dCase fs.DirEntry, err error) error {
		if err != nil {
			return nil
		}
		if dCase.IsDir() {
			if dCase.Name() == "CMakeFiles" || dCase.Name() == ".cmake" {
				return filepath.SkipDir
			}
			return nil
		}
		name := dCase.Name()
		if !strings.HasSuffix(name, suffix) {
			return nil
		}
		info, e := dCase.Info()
		if e != nil || info.Mode()&0o111 == 0 {
			return nil
		}
		rel, _ := filepath.Rel(buildDir, path)
		found = rel
		return filepath.SkipAll
	})
	if err != nil {
		return "", err
	}
	if found == "" {
		return "", fmt.Errorf("executable for case %q not found", caseName)
	}
	return found, nil
}
