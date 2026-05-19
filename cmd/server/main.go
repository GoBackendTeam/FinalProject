package main

import (
	"context"
	"errors"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/api"
	"github.com/GoBackendTeam/FinalProject/internal/api/handler"
	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/GoBackendTeam/FinalProject/internal/config"
	"github.com/GoBackendTeam/FinalProject/internal/judge"
	"github.com/GoBackendTeam/FinalProject/internal/store"
	"github.com/joho/godotenv"
)

func main() {
	_ = godotenv.Load()

	cfg, err := config.Load()
	if err != nil {
		log.Fatalf("config: %v", err)
	}

	jm, err := auth.NewManager(cfg.JWTPrivateKeyPath, cfg.JWTPublicKeyPath, cfg.JWTTTL)
	if err != nil {
		log.Fatalf("jwt: %v (先執行 `make keys` 產生金鑰)", err)
	}

	st := mustOpenDB(cfg.DatabaseURL)
	if err := st.SeedProblems(cfg.ProblemsRoot); err != nil {
		log.Printf("seed problems: %v", err)
	}
	adminUser := getenv("ADMIN_USERNAME", "admin")
	adminPass := getenv("ADMIN_PASSWORD", "admin")
	if err := st.EnsureAdmin(adminUser, adminPass); err != nil {
		log.Printf("ensure admin: %v", err)
	} else {
		log.Printf("[init] admin 帳號就緒(帳號=%s)", adminUser)
	}

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	eng, err := judge.NewEngine(cfg, st)
	if err != nil {
		log.Fatalf("judge engine: %v", err)
	}
	eng.Start(ctx)

	h := handler.New(cfg, st, jm, eng)
	srv := &http.Server{Addr: cfg.HTTPAddr, Handler: api.NewRouter(h, jm)}

	go func() {
		log.Printf("[http] listening on %s", cfg.HTTPAddr)
		if err := srv.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.Fatalf("http server: %v", err)
		}
	}()

	<-ctx.Done()
	log.Println("[shutdown] 收到終止訊號,優雅關閉中…")
	shutCtx, cancel := context.WithTimeout(context.Background(), 15*time.Second)
	defer cancel()
	_ = srv.Shutdown(shutCtx)
	eng.Stop()
	log.Println("[shutdown] done")
}

func mustOpenDB(dsn string) *store.Store {
	var lastErr error
	for i := 0; i < 15; i++ {
		st, err := store.Open(dsn)
		if err == nil {
			return st
		}
		lastErr = err
		log.Printf("[db] 連線失敗(第 %d 次),2 秒後重試: %v", i+1, err)
		time.Sleep(2 * time.Second)
	}
	log.Fatalf("[db] 無法連線: %v", lastErr)
	return nil
}

func getenv(k, def string) string {
	if v := os.Getenv(k); v != "" {
		return v
	}
	return def
}
