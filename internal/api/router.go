package api

import (
	"net/http"

	"github.com/GoBackendTeam/FinalProject/docs"
	"github.com/GoBackendTeam/FinalProject/internal/api/handler"
	"github.com/GoBackendTeam/FinalProject/internal/api/middleware"
	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/gin-gonic/gin"
)

func NewRouter(h *handler.Handler, jm *auth.Manager) *gin.Engine {
	r := gin.New()
	r.Use(gin.Logger(), gin.Recovery())
	r.MaxMultipartMemory = 32 << 20

	r.GET("/healthz", func(c *gin.Context) { c.JSON(http.StatusOK, gin.H{"status": "ok"}) })

	// ---- API 文件(規格 + 互動式 UI),皆為 Guest 可存取 ----
	// 規格以 go:embed 打包進 binary,單一真相來源為 docs/openapi.yaml。
	r.GET("/openapi.yaml", func(c *gin.Context) {
		c.Data(http.StatusOK, "application/yaml; charset=utf-8", docs.OpenAPIYAML)
	})
	// /docs:Scalar 互動式文件(內建 API console),從 /openapi.yaml 載入規格。
	r.GET("/docs", func(c *gin.Context) {
		c.Data(http.StatusOK, "text/html; charset=utf-8", docs.ScalarHTML)
	})

	api := r.Group("/api")

	user := middleware.RequireRole(jm, model.RoleUser)
	admin := middleware.RequireRole(jm, model.RoleAdmin)

	// ---- UserInfo ----
	api.POST("/users/register", h.Register)                   // Guest
	api.POST("/users/login", h.Login)                         // Guest
	api.POST("/users/logout", user, h.Logout)                 // User
	api.GET("/users/me", user, h.Me)                          // User
	api.GET("/users/:user_id/submissions", h.UserSubmissions) // Guest

	// ---- Problem ----
	api.GET("/problems", h.ListProblems)                                   // Guest
	api.GET("/problems/:problem_id", h.GetProblem)                         // Guest
	api.PUT("/problems", admin, h.PutProblem)                              // Admin
	api.DELETE("/problems/:problem_id", admin, h.DeleteProblem)            // Admin
	api.GET("/problems/:problem_id/testcases", admin, h.DownloadTestcases) // Admin

	// ---- Submission ----
	api.POST("/submissions", user, h.CreateSubmission)                        // User
	api.GET("/submissions", user, h.ListSubmissions)                          // User
	api.GET("/submissions/:operatorId", user, h.GetSubmission)                // User
	api.GET("/submissions/:operatorId/source", user, h.GetSubmissionSource)   // User
	api.GET("/submissions/:operatorId/logs/:stage", user, h.GetSubmissionLog) // User(多分段日誌)
	api.POST("/submissions/:operatorId/rerun", user, h.RerunSubmission)       // User/Admin(重跑 Job)

	// ---- Statistics ----
	api.GET("/stats/problems/:problem_id", h.ProblemStats) // Guest
	api.GET("/stats/users/:user_id", h.UserStats)          // Guest

	return r
}
