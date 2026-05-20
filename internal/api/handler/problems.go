package handler

import (
	"archive/zip"
	"fmt"
	"io"
	"io/fs"
	"log"
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
		if fh.Size > MaxUploadBytes {
			c.JSON(http.StatusRequestEntityTooLarge,
				gin.H{"error": fmt.Sprintf("archive exceeds %d bytes", MaxUploadBytes)})
			return
		}
		dst := filepath.Join(h.Cfg.ProblemsRoot, id)
		nonce := time.Now().UnixNano()
		tmp := filepath.Join(h.Cfg.ProblemsRoot, fmt.Sprintf(".upload-%s-%d%s", id, nonce, filepath.Ext(fh.Filename)))
		staging := filepath.Join(h.Cfg.ProblemsRoot, fmt.Sprintf(".staging-%s-%d", id, nonce))
		// 不論成功與否都清掉暫存;dst 只有在驗證通過後才會被動到。
		defer os.Remove(tmp)
		defer os.RemoveAll(staging)

		if err := os.MkdirAll(staging, 0o755); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "mkdir staging"})
			return
		}
		if err := c.SaveUploadedFile(fh, tmp); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "save archive"})
			return
		}
		// 先解壓到 staging 並驗證,壞包不會影響既有題目。
		if err := archive.Extract(tmp, staging); err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "extract archive: " + err.Error()})
			return
		}
		if !archive.HasCMakeLists(staging) {
			c.JSON(http.StatusBadRequest, gin.H{"error": "題目壓縮檔根目錄缺少 CMakeLists.txt"})
			return
		}
		cases := discoverSpecCases(staging)
		if len(cases) == 0 {
			c.JSON(http.StatusBadRequest, gin.H{"error": "題目缺少 spec/ 測資(無 case)"})
			return
		}

		// 驗證通過 → 原子替換:舊題先移到 backup,失敗可回滾。
		backup := filepath.Join(h.Cfg.ProblemsRoot, fmt.Sprintf(".bak-%s-%d", id, nonce))
		if _, statErr := os.Stat(dst); statErr == nil {
			if err := os.Rename(dst, backup); err != nil {
				c.JSON(http.StatusInternalServerError, gin.H{"error": "swap out old problem"})
				return
			}
		}
		if err := os.Rename(staging, dst); err != nil {
			_ = os.Rename(backup, dst) // 回滾
			c.JSON(http.StatusInternalServerError, gin.H{"error": "install new problem"})
			return
		}
		_ = os.RemoveAll(backup)

		p.ProblemDir = dst
		p.ArchivePath = filepath.Join(h.Cfg.ProblemsRoot, id+"-package"+filepath.Ext(fh.Filename))
		_ = copyFile(tmp, p.ArchivePath)
		p.CaseNames = strings.Join(cases, ",")
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
// 若有 admin 上傳的壓縮檔(ArchivePath)直接回;沒有(例如從 PROBLEMS_ROOT
// 自動 seed 的題目)就即時把 ProblemDir 打包成 zip 串流回傳。
func (h *Handler) DownloadTestcases(c *gin.Context) {
	var p model.Problem
	if err := h.Store.DB.First(&p, "id = ?", c.Param("problem_id")).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "problem not found"})
		return
	}
	if p.ArchivePath != "" && fileExists(p.ArchivePath) {
		c.FileAttachment(p.ArchivePath, p.ID+filepath.Ext(p.ArchivePath))
		return
	}
	if p.ProblemDir == "" {
		c.JSON(http.StatusNotFound, gin.H{"error": "no archive or problem directory available"})
		return
	}
	streamProblemDirAsZip(c, p.ID, p.ProblemDir)
}

func streamProblemDirAsZip(c *gin.Context, problemID, dir string) {
	c.Header("Content-Type", "application/zip")
	c.Header("Content-Disposition", fmt.Sprintf(`attachment; filename="%s.zip"`, problemID))
	zw := zip.NewWriter(c.Writer)
	defer zw.Close()
	base := filepath.Clean(dir)
	err := filepath.WalkDir(base, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if d.IsDir() {
			return nil
		}
		name := d.Name()
		if name == ".DS_Store" || strings.HasPrefix(name, "._") {
			return nil
		}
		rel, err := filepath.Rel(base, path)
		if err != nil {
			return err
		}
		w, err := zw.Create(filepath.ToSlash(rel))
		if err != nil {
			return err
		}
		f, err := os.Open(path)
		if err != nil {
			return err
		}
		defer f.Close()
		_, err = io.Copy(w, f)
		return err
	})
	if err != nil {
		// headers 已送出,只能 log。
		log.Printf("[problems] stream zip %s: %v", problemID, err)
	}
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
