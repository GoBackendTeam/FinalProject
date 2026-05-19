package handler

import (
	"net/http"

	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/gin-gonic/gin"
)

type statusCount struct {
	Status model.Status `json:"status"`
	Count  int64        `json:"count"`
}

func (h *Handler) statusBreakdown(field, value string) (gin.H, error) {
	var rows []statusCount
	err := h.Store.DB.Model(&model.Submission{}).
		Select("status, count(*) as count").
		Where(field+" = ?", value).
		Group("status").Scan(&rows).Error
	if err != nil {
		return nil, err
	}
	var total int64
	byStatus := gin.H{}
	for _, r := range rows {
		byStatus[string(r.Status)] = r.Count
		total += r.Count
	}
	return gin.H{"total": total, "by_status": byStatus}, nil
}

func (h *Handler) ProblemStats(c *gin.Context) {
	pid := c.Param("problem_id")
	res, err := h.statusBreakdown("problem_id", pid)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "stats failed"})
		return
	}
	res["problem_id"] = pid
	c.JSON(http.StatusOK, res)
}

func (h *Handler) UserStats(c *gin.Context) {
	uid := c.Param("user_id")
	res, err := h.statusBreakdown("user_id", uid)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "stats failed"})
		return
	}
	res["user_id"] = uid
	c.JSON(http.StatusOK, res)
}
