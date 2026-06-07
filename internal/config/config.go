package config

import (
	"os"
	"path/filepath"
	"strconv"
	"time"
)

// Config 為整個服務的執行期設定,全部來自環境變數(見 .env.example)。
type Config struct {
	HTTPAddr string

	DatabaseURL string

	JWTPrivateKeyPath string
	JWTPublicKeyPath  string
	JWTTTL            time.Duration

	ProblemsRoot       string
	JudgeWorkspace     string // app 視角的工作目錄
	JudgeWorkspaceHost string // 對應的主機路徑(bind mount 用)
	JudgeImage         string
	JudgePlatform      string
	JudgeMaxConcurrency int
	JudgeCaseTimeout   time.Duration
	JudgeBuildTimeout  time.Duration
	JudgeBuildNetwork  string

	// OpenAPISpecPath 為 OpenAPI YAML 的路徑,由 /docs 互動文件載入。
	OpenAPISpecPath string
}

func env(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}

func envInt(key string, def int) int {
	if v := os.Getenv(key); v != "" {
		if n, err := strconv.Atoi(v); err == nil {
			return n
		}
	}
	return def
}

// Load 讀取環境變數並補上預設值與絕對路徑。
func Load() (*Config, error) {
	c := &Config{
		HTTPAddr:            env("HTTP_ADDR", ":8080"),
		DatabaseURL:         env("DATABASE_URL", "postgres://regs:regs@localhost:5432/regs?sslmode=disable"),
		JWTPrivateKeyPath:   env("JWT_PRIVATE_KEY_PATH", "./keys/private.pem"),
		JWTPublicKeyPath:    env("JWT_PUBLIC_KEY_PATH", "./keys/public.pem"),
		JWTTTL:              time.Duration(envInt("JWT_TTL_HOURS", 12)) * time.Hour,
		ProblemsRoot:        env("PROBLEMS_ROOT", "./problems"),
		JudgeWorkspace:      env("JUDGE_WORKSPACE", "./workspace"),
		JudgeWorkspaceHost:  env("JUDGE_WORKSPACE_HOST", ""),
		JudgeImage:          env("JUDGE_IMAGE", "yhlib/cs3060701"),
		JudgePlatform:       env("JUDGE_PLATFORM", "linux/amd64"),
		JudgeMaxConcurrency: envInt("JUDGE_MAX_CONCURRENCY", 2),
		JudgeCaseTimeout:    time.Duration(envInt("JUDGE_CASE_TIMEOUT", 10)) * time.Second,
		JudgeBuildTimeout:   time.Duration(envInt("JUDGE_BUILD_TIMEOUT", 120)) * time.Second,
		JudgeBuildNetwork:   env("JUDGE_BUILD_NETWORK", "regs-build"),
		OpenAPISpecPath:     env("OPENAPI_SPEC_PATH", "./docs/openapi.yaml"),
	}

	abs := func(p string) string {
		if a, err := filepath.Abs(p); err == nil {
			return a
		}
		return p
	}
	c.ProblemsRoot = abs(c.ProblemsRoot)
	c.JudgeWorkspace = abs(c.JudgeWorkspace)
	// app 直接跑在主機時,主機路徑等於 app 路徑。
	if c.JudgeWorkspaceHost == "" {
		c.JudgeWorkspaceHost = c.JudgeWorkspace
	}
	if c.JudgeMaxConcurrency < 1 {
		c.JudgeMaxConcurrency = 1
	}
	return c, nil
}
