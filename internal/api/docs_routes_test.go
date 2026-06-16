package api

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/GoBackendTeam/FinalProject/internal/api/handler"
	"github.com/gin-gonic/gin"
)

// TestDocRoutes 驗證文件端點正確掛載並回傳預期內容(不需 DB/Docker)。
func TestDocRoutes(t *testing.T) {
	gin.SetMode(gin.TestMode)
	r := NewRouter(&handler.Handler{}, nil)

	cases := []struct {
		path        string
		wantStatus  int
		wantCType   string
		wantContain string
	}{
		{"/openapi.yaml", 200, "application/yaml", "openapi: 3.0.3"},
		{"/docs", 200, "text/html", "api-reference"}, // Scalar
	}
	for _, c := range cases {
		req := httptest.NewRequest(http.MethodGet, c.path, nil)
		w := httptest.NewRecorder()
		r.ServeHTTP(w, req)
		if w.Code != c.wantStatus {
			t.Errorf("%s: status = %d, want %d", c.path, w.Code, c.wantStatus)
		}
		if c.wantCType != "" && !strings.Contains(w.Header().Get("Content-Type"), c.wantCType) {
			t.Errorf("%s: content-type = %q, want contains %q", c.path, w.Header().Get("Content-Type"), c.wantCType)
		}
		if c.wantContain != "" && !strings.Contains(w.Body.String(), c.wantContain) {
			t.Errorf("%s: body missing %q", c.path, c.wantContain)
		}
	}
}
