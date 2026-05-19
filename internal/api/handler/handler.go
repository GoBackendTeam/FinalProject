package handler

import (
	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/GoBackendTeam/FinalProject/internal/config"
	"github.com/GoBackendTeam/FinalProject/internal/judge"
	"github.com/GoBackendTeam/FinalProject/internal/store"
)

type Handler struct {
	Cfg    *config.Config
	Store  *store.Store
	JWT    *auth.Manager
	Engine *judge.Engine
}

func New(cfg *config.Config, st *store.Store, jm *auth.Manager, eng *judge.Engine) *Handler {
	return &Handler{Cfg: cfg, Store: st, JWT: jm, Engine: eng}
}
