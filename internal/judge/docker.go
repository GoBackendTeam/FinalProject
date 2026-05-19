package judge

import (
	"context"
	"fmt"
	"io"
	"strings"
	"time"

	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/image"
	"github.com/docker/docker/api/types/network"
	"github.com/docker/docker/client"
	ocispec "github.com/opencontainers/image-spec/specs-go/v1"
)

// dockerRunner 封裝判題所需的 Docker Engine API 呼叫。
type dockerRunner struct {
	cli          *client.Client
	platformStr  string           // 例如 "linux/amd64"(空字串=daemon 預設)
	platform     *ocispec.Platform // 給 ContainerCreate 用
}

func newDockerRunner(platform string) (*dockerRunner, error) {
	cli, err := client.NewClientWithOpts(client.FromEnv, client.WithAPIVersionNegotiation())
	if err != nil {
		return nil, err
	}
	d := &dockerRunner{cli: cli, platformStr: platform}
	if platform != "" {
		parts := strings.SplitN(platform, "/", 3)
		p := &ocispec.Platform{OS: parts[0]}
		if len(parts) > 1 {
			p.Architecture = parts[1]
		}
		if len(parts) > 2 {
			p.Variant = parts[2]
		}
		d.platform = p
	}
	return d, nil
}

func (d *dockerRunner) ping(ctx context.Context) error {
	_, err := d.cli.Ping(ctx)
	return err
}

// ensureNetwork 確保編譯容器用的 bridge network 存在(與執行容器的 none 分屬不同網段)。
func (d *dockerRunner) ensureNetwork(ctx context.Context, name string) error {
	_, err := d.cli.NetworkCreate(ctx, name, network.CreateOptions{Driver: "bridge"})
	if err != nil && !strings.Contains(err.Error(), "already exists") {
		return err
	}
	return nil
}

// ensureImage 確保評測映像已存在,不存在則嘗試 pull(best-effort)。
func (d *dockerRunner) ensureImage(ctx context.Context, ref string) error {
	if _, _, err := d.cli.ImageInspectWithRaw(ctx, ref); err == nil {
		return nil
	}
	rc, err := d.cli.ImagePull(ctx, ref, image.PullOptions{Platform: d.platformStr})
	if err != nil {
		return err
	}
	defer rc.Close()
	_, _ = io.Copy(io.Discard, rc) // 等 pull 完成
	return nil
}

type runResult struct {
	ExitCode  int
	TimedOut  bool
	OOMKilled bool
}

type runSpec struct {
	Image       string
	Cmd         []string
	Binds       []string // host:container[:ro]
	NetworkMode string   // build network 名稱,或 "none"
	WorkingDir  string   // 預設 "/"
	Timeout     time.Duration

	// 資源與能力加固(對抗惡意提交)
	MemoryBytes     int64
	NanoCPUs        int64
	PidsLimit       int64             // > 0 才套用,擋 fork bomb
	ReadonlyRootfs  bool              // 唯讀根檔系統
	DropAllCaps     bool              // CapDrop: ALL
	NoNewPrivileges bool              // no-new-privileges
	Tmpfs           map[string]string // 唯讀 rootfs 下提供可寫暫存
}

// run 建立容器、啟動、等待結束(含逾時主動終止),回傳結束碼與旗標。
func (d *dockerRunner) run(ctx context.Context, spec runSpec) (runResult, error) {
	var res runResult

	workDir := spec.WorkingDir
	if workDir == "" {
		workDir = "/"
	}
	cfg := &container.Config{
		Image:        spec.Image,
		Cmd:          spec.Cmd,
		WorkingDir:   workDir,
		Tty:          false,
		AttachStdout: false,
		AttachStderr: false,
	}
	resCfg := container.Resources{
		Memory:   spec.MemoryBytes,
		NanoCPUs: spec.NanoCPUs,
	}
	if spec.PidsLimit > 0 {
		pl := spec.PidsLimit
		resCfg.PidsLimit = &pl
	}
	host := &container.HostConfig{
		Binds:          spec.Binds,
		NetworkMode:    container.NetworkMode(spec.NetworkMode),
		AutoRemove:     false,
		ReadonlyRootfs: spec.ReadonlyRootfs,
		Resources:      resCfg,
	}
	if spec.DropAllCaps {
		host.CapDrop = []string{"ALL"}
	}
	if spec.NoNewPrivileges {
		host.SecurityOpt = append(host.SecurityOpt, "no-new-privileges:true")
	}
	if len(spec.Tmpfs) > 0 {
		host.Tmpfs = spec.Tmpfs
	}

	created, err := d.cli.ContainerCreate(ctx, cfg, host, nil, d.platform, "")
	if err != nil {
		return res, fmt.Errorf("container create: %w", err)
	}
	id := created.ID
	// 清理用獨立 context,避免逾時的父 context 連帶取消清理。
	defer func() {
		clean, cancel := context.WithTimeout(context.Background(), 15*time.Second)
		defer cancel()
		_ = d.cli.ContainerRemove(clean, id, container.RemoveOptions{Force: true})
	}()

	if err := d.cli.ContainerStart(ctx, id, container.StartOptions{}); err != nil {
		return res, fmt.Errorf("container start: %w", err)
	}

	waitCtx := ctx
	var cancel context.CancelFunc
	if spec.Timeout > 0 {
		waitCtx, cancel = context.WithTimeout(ctx, spec.Timeout)
		defer cancel()
	}

	statusCh, errCh := d.cli.ContainerWait(waitCtx, id, container.WaitConditionNotRunning)
	select {
	case <-waitCtx.Done():
		// 逾時:主動終止容器並標記 TLE。
		killCtx, kc := context.WithTimeout(context.Background(), 10*time.Second)
		defer kc()
		_ = d.cli.ContainerKill(killCtx, id, "KILL")
		res.TimedOut = true
		return res, nil
	case err := <-errCh:
		if err != nil {
			return res, fmt.Errorf("container wait: %w", err)
		}
	case <-statusCh:
	}

	inspect, err := d.cli.ContainerInspect(context.Background(), id)
	if err != nil {
		return res, fmt.Errorf("container inspect: %w", err)
	}
	if inspect.State != nil {
		res.ExitCode = inspect.State.ExitCode
		res.OOMKilled = inspect.State.OOMKilled
	}
	return res, nil
}
