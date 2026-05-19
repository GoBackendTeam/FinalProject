package judge

import "github.com/GoBackendTeam/FinalProject/internal/model"

// classifyCase 依容器執行結果判定單一 case 狀態。
//
// 判題框架是 Catch2 + TAP:session.run() 回傳失敗測試數作為 exit code。
//   - timedOut          → TLE(逾時被我們主動終止)
//   - oomKilled         → RE(資源耗盡視為執行錯誤)
//   - exitCode == 0     → AC
//   - exitCode >= 128   → RE(被訊號終止,如 SIGSEGV=139、SIGABRT=134)
//   - 其餘 (1..127)     → WA(Catch2 回報的失敗測試數)
func classifyCase(exitCode int, timedOut, oomKilled bool) model.Status {
	switch {
	case timedOut:
		return model.StatusTLE
	case oomKilled:
		return model.StatusRE
	case exitCode == 0:
		return model.StatusAC
	case exitCode >= 128:
		return model.StatusRE
	default:
		return model.StatusWA
	}
}

// aggregate 依各 case 結果彙整出整體提交狀態。
// 嚴重度:RE > TLE > WA > AC;只要全 AC 才是 AC。
func aggregate(results []model.CaseResult) model.Status {
	if len(results) == 0 {
		return model.StatusIE
	}
	hasRE, hasTLE, hasWA := false, false, false
	for _, r := range results {
		switch r.Status {
		case model.StatusRE:
			hasRE = true
		case model.StatusTLE:
			hasTLE = true
		case model.StatusWA:
			hasWA = true
		}
	}
	switch {
	case hasRE:
		return model.StatusRE
	case hasTLE:
		return model.StatusTLE
	case hasWA:
		return model.StatusWA
	default:
		return model.StatusAC
	}
}
