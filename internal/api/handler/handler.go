package handler

import (
	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/GoBackendTeam/FinalProject/internal/config"
	"github.com/GoBackendTeam/FinalProject/internal/judge"
	"github.com/GoBackendTeam/FinalProject/internal/store"
)

// MaxUploadBytes 為單次上傳壓縮檔的硬上限(擋大檔吃爆磁碟)。
const MaxUploadBytes = 100 << 20

type Handler struct {
	Cfg    *config.Config
	Store  *store.Store
	JWT    *auth.Manager
	Engine *judge.Engine
}

func New(cfg *config.Config, st *store.Store, jm *auth.Manager, eng *judge.Engine) *Handler {
	return &Handler{Cfg: cfg, Store: st, JWT: jm, Engine: eng}
}
