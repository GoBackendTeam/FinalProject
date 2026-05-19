package middleware

import (
	"net/http"
	"strings"

	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/gin-gonic/gin"
)

const ClaimsKey = "claims"

// Optional 嘗試解析 JWT;沒帶或無效就當 Guest,不擋下請求。
func Optional(jm *auth.Manager) gin.HandlerFunc {
	return func(c *gin.Context) {
		if tok := bearer(c); tok != "" {
			if cl, err := jm.Verify(tok); err == nil {
				c.Set(ClaimsKey, cl)
			}
		}
		c.Next()
	}
}

// RequireRole 要求請求帶有合法 JWT,且角色 >= min。
func RequireRole(jm *auth.Manager, min model.Role) gin.HandlerFunc {
	return func(c *gin.Context) {
		tok := bearer(c)
		if tok == "" {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "missing Authorization header"})
			return
		}
		cl, err := jm.Verify(tok)
		if err != nil {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "invalid token"})
			return
		}
		if rank(cl.Role) < rank(min) {
			c.AbortWithStatusJSON(http.StatusForbidden, gin.H{"error": "insufficient role"})
			return
		}
		c.Set(ClaimsKey, cl)
		c.Next()
	}
}

func bearer(c *gin.Context) string {
	h := c.GetHeader("Authorization")
	if h == "" {
		return ""
	}
	if strings.HasPrefix(strings.ToLower(h), "bearer ") {
		return strings.TrimSpace(h[7:])
	}
	return strings.TrimSpace(h)
}

// rank:Guest < User < Admin。
func rank(r model.Role) int {
	switch r {
	case model.RoleAdmin:
		return 3
	case model.RoleUser:
		return 2
	default:
		return 1
	}
}

// Current 取出已驗證的使用者(沒有則回 nil)。
func Current(c *gin.Context) *auth.Claims {
	if v, ok := c.Get(ClaimsKey); ok {
		if cl, ok := v.(*auth.Claims); ok {
			return cl
		}
	}
	return nil
}
