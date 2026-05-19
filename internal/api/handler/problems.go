package handler

import (
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/archive"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/gin-gonic/gin"
)

func (h *Handler) ListProblems(c *gin.Context) {
	var ps []model.Problem
	h.Store.DB.Order("id").Find(&ps)
	c.JSON(http.StatusOK, gin.H{"problems": ps})
}

func (h *Handler) GetProblem(c *gin.Context) {
	var p model.Problem
	if err := h.Store.DB.First(&p, "id = ?", c.Param("problem_id")).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "problem not found"})
		return
	}
	c.JSON(http.StatusOK, gin.H{
		"id": p.ID, "title": p.Title, "description": p.Description,
		"cases": strings.Split(p.CaseNames, ","),
	})
}

type putProblemReq struct {
	ID          string `json:"id" binding:"required"`
	Title       string `json:"title"`
	Description string `json:"description"`
}

// PutProblem:Admin 建立或更新題目 metadata。
// 若同時帶 multipart 檔案(欄位 archive),會解壓題目壓縮檔至題庫並重新登錄 case。
func (h *Handler) PutProblem(c *gin.Context) {
	id := c.PostForm("id")
	var title, desc string
	if id != "" { // multipart 路徑
		title = c.PostForm("title")
		desc = c.PostForm("description")
	} else { // JSON 路徑
		var req putProblemReq
		if err := c.ShouldBindJSON(&req); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
			return
		}
		id, title, desc = req.ID, req.Title, req.Description
	}

	var p model.Problem
	err := h.Store.DB.First(&p, "id = ?", id).Error
	isNew := err != nil
	p.ID = id
	if title != "" {
		p.Title = title
	}
	if desc != "" {
		p.Description = desc
	}

	if fh, ferr := c.FormFile("archive"); ferr == nil {
		dst := filepath.Join(h.Cfg.ProblemsRoot, id)
		_ = os.RemoveAll(dst)
		if err := os.MkdirAll(dst, 0o755); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "mkdir problem dir"})
			return
		}
		tmp := filepath.Join(h.Cfg.ProblemsRoot, "."+id+filepath.Ext(fh.Filename))
		if err := c.SaveUploadedFile(fh, tmp); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "save archive"})
			return
		}
		defer os.Remove(tmp)
		if err := archive.Extract(tmp, dst); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "extract archive: " + err.Error()})
			return
		}
		if !archive.HasCMakeLists(dst) {
			c.JSON(http.StatusBadRequest, gin.H{"error": "題目壓縮檔根目錄缺少 CMakeLists.txt"})
			return
		}
		p.ProblemDir = dst
		p.ArchivePath = filepath.Join(h.Cfg.ProblemsRoot, id+"-package"+filepath.Ext(fh.Filename))
		_ = copyFile(tmp, p.ArchivePath)
		p.CaseNames = strings.Join(discoverSpecCases(dst), ",")
	}

	now := time.Now()
	p.UpdatedAt = now
	if isNew {
		p.CreatedAt = now
		if err := h.Store.DB.Create(&p).Error; err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "create failed"})
			return
		}
		c.JSON(http.StatusCreated, p)
		return
	}
	h.Store.DB.Save(&p)
	c.JSON(http.StatusOK, p)
}

func (h *Handler) DeleteProblem(c *gin.Context) {
	id := c.Param("problem_id")
	if err := h.Store.DB.Delete(&model.Problem{}, "id = ?", id).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "delete failed"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "deleted", "id": id})
}

// DownloadTestcases:Admin 下載題目壓縮檔。
func (h *Handler) DownloadTestcases(c *gin.Context) {
	var p model.Problem
	if err := h.Store.DB.First(&p, "id = ?", c.Param("problem_id")).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "problem not found"})
		return
	}
	if p.ArchivePath == "" || !fileExists(p.ArchivePath) {
		c.JSON(http.StatusNotFound, gin.H{"error": "no archive stored for this problem"})
		return
	}
	c.FileAttachment(p.ArchivePath, p.ID+filepath.Ext(p.ArchivePath))
}

func discoverSpecCases(problemDir string) []string {
	entries, err := os.ReadDir(filepath.Join(problemDir, "spec"))
	if err != nil {
		return nil
	}
	var cs []string
	for _, e := range entries {
		if !e.IsDir() && strings.HasSuffix(e.Name(), ".h") {
			cs = append(cs, strings.TrimSuffix(e.Name(), ".h"))
		}
	}
	return cs
}
