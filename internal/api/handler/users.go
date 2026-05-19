package handler

import (
	"net/http"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/api/middleware"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/GoBackendTeam/FinalProject/internal/store"
	"github.com/gin-gonic/gin"
	"golang.org/x/crypto/bcrypt"
	"gorm.io/gorm"
)

type registerReq struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required,min=4"`
}

func (h *Handler) Register(c *gin.Context) {
	var req registerReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	var n int64
	h.Store.DB.Model(&model.User{}).Where("username = ?", req.Username).Count(&n)
	if n > 0 {
		c.JSON(http.StatusConflict, gin.H{"error": "username taken"})
		return
	}
	hash, _ := bcrypt.GenerateFromPassword([]byte(req.Password), bcrypt.DefaultCost)
	u := model.User{
		ID:           store.NewID(),
		Username:     req.Username,
		PasswordHash: string(hash),
		Role:         model.RoleUser,
		CreatedAt:    time.Now(),
	}
	if err := h.Store.DB.Create(&u).Error; err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "create user failed"})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"id": u.ID, "username": u.Username, "role": u.Role})
}

type loginReq struct {
	Username string `json:"username" binding:"required"`
	Password string `json:"password" binding:"required"`
}

func (h *Handler) Login(c *gin.Context) {
	var req loginReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	var u model.User
	if err := h.Store.DB.First(&u, "username = ?", req.Username).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "invalid credentials"})
		return
	}
	if bcrypt.CompareHashAndPassword([]byte(u.PasswordHash), []byte(req.Password)) != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "invalid credentials"})
		return
	}
	tok, err := h.JWT.Sign(&u)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "sign token failed"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"token": tok, "token_type": "Bearer",
		"user": gin.H{"id": u.ID, "username": u.Username, "role": u.Role}})
}

// Logout:JWT 為無狀態,登出由客戶端丟棄 token 即可。
func (h *Handler) Logout(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{"message": "logged out; discard the token client-side"})
}

func (h *Handler) Me(c *gin.Context) {
	cl := middleware.Current(c)
	var u model.User
	if err := h.Store.DB.First(&u, "id = ?", cl.UserID).Error; err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "user not found"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"id": u.ID, "username": u.Username,
		"role": u.Role, "created_at": u.CreatedAt})
}

// UserSubmissions:Guest 可查任意使用者的提交紀錄列表。
func (h *Handler) UserSubmissions(c *gin.Context) {
	uid := c.Param("user_id")
	var subs []model.Submission
	if err := h.Store.DB.Where("user_id = ?", uid).
		Order("created_at desc").Find(&subs).Error; err != nil && err != gorm.ErrRecordNotFound {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "query failed"})
		return
	}
	c.JSON(http.StatusOK, gin.H{"submissions": subs})
}
