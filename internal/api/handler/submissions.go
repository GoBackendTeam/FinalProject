package handler

import (
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/api/middleware"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/GoBackendTeam/FinalProject/internal/store"
	"github.com/gin-gonic/gin"
)

// CreateSubmission:User 上傳專案壓縮檔提交評測。
// 立即回傳 operatorId(202),評測在背景進行(對應 PRD 非同步要求)。
func (h *Handler) CreateSubmission(c *gin.Context) {
	cl := middleware.Current(c)
	problemID := c.PostForm("problem_id")
	if problemID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "problem_id is required"})
		return
	}
	var prob model.Problem
	if err := h.Store.DB.First(&prob, "id = ?", problemID).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "problem not found"})
		return
	}
	fh, err := c.FormFile("file")
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "file is required (zip/tar/tgz)"})
		return
	}
	ext := strings.ToLower(filepath.Ext(fh.Filename))
	if low := strings.ToLower(fh.Filename); strings.HasSuffix(low, ".tar.gz") {
		ext = ".tar.gz"
	}
	if ext == "" {
		ext = ".zip"
	}

	id := store.NewID()
	base := filepath.Join(h.Cfg.JudgeWorkspace, id)
	if err := os.MkdirAll(base, 0o755); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "prepare workspace failed"})
		return
	}
	archivePath := filepath.Join(base, "upload"+ext)
	if err := c.SaveUploadedFile(fh, archivePath); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "save upload failed"})
		return
	}

	sub := model.Submission{
		ID:                id,
		UserID:            cl.UserID,
		ProblemID:         problemID,
		Status:            model.StatusPending,
		SourceArchivePath: archivePath,
		WorkspaceDir:      base,
		CreatedAt:         time.Now(),
	}
	if err := h.Store.DB.Create(&sub).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "create submission failed"})
		return
	}

	h.Engine.Enqueue(id)
	c.JSON(http.StatusAccepted, gin.H{"operatorId": id, "status": sub.Status})
}

// ListSubmissions:User 查詢自己的提交紀錄列表。
func (h *Handler) ListSubmissions(c *gin.Context) {
	cl := middleware.Current(c)
	var subs []model.Submission
	h.Store.DB.Where("user_id = ?", cl.UserID).Order("created_at desc").Find(&subs)
	c.JSON(http.StatusOK, gin.H{"submissions": subs})
}

func (h *Handler) loadOwned(c *gin.Context) (*model.Submission, bool) {
	cl := middleware.Current(c)
	var sub model.Submission
	if err := h.Store.DB.Preload("CaseResults").
		First(&sub, "id = ?", c.Param("operatorId")).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "submission not found"})
		return nil, false
	}
	// User 只能看自己的;Admin 可看任意人(對應「查看他人代碼」)。
	if sub.UserID != cl.UserID && cl.Role != model.RoleAdmin {
		c.JSON(http.StatusForbidden, gin.H{"error": "not your submission"})
		return nil, false
	}
	return &sub, true
}

// GetSubmission:取得單一提交的評測結果。
func (h *Handler) GetSubmission(c *gin.Context) {
	sub, ok := h.loadOwned(c)
	if !ok {
		return
	}
	c.JSON(http.StatusOK, sub)
}

// GetSubmissionSource:下載提交的原始專案檔。
func (h *Handler) GetSubmissionSource(c *gin.Context) {
	sub, ok := h.loadOwned(c)
	if !ok {
		return
	}
	if sub.SourceArchivePath == "" || !fileExists(sub.SourceArchivePath) {
		c.JSON(http.StatusNotFound, gin.H{"error": "source archive missing"})
		return
	}
	c.FileAttachment(sub.SourceArchivePath, sub.ID+filepath.Ext(sub.SourceArchivePath))
}

// GetSubmissionLog:多分段日誌查詢(stage = configure|compile|output)。
func (h *Handler) GetSubmissionLog(c *gin.Context) {
	sub, ok := h.loadOwned(c)
	if !ok {
		return
	}
	var path string
	switch c.Param("stage") {
	case "configure":
		path = sub.ConfigureLogPath
	case "compile":
		path = sub.CompileLogPath
	case "output":
		path = sub.OutputLogPath
	default:
		c.JSON(http.StatusBadRequest, gin.H{"error": "stage must be configure|compile|output"})
		return
	}
	if path == "" || !fileExists(path) {
		c.JSON(http.StatusOK, gin.H{"stage": c.Param("stage"), "log": ""})
		return
	}
	data, err := os.ReadFile(path)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "read log failed"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"stage": c.Param("stage"), "log": string(data)})
}

// RerunSubmission:重新執行某個 Job(Admin,或本人)。
func (h *Handler) RerunSubmission(c *gin.Context) {
	sub, ok := h.loadOwned(c)
	if !ok {
		return
	}
	if err := h.Engine.Rerun(sub.ID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "rerun failed"})
		return
	}
	c.JSON(http.StatusAccepted, gin.H{"operatorId": sub.ID, "status": model.StatusPending})
}
