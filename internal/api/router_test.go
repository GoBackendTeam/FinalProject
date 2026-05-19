package api

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"encoding/pem"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/api/handler"
	"github.com/GoBackendTeam/FinalProject/internal/auth"
	"github.com/gin-gonic/gin"
)

func testManager(t *testing.T) *auth.Manager {
	t.Helper()
	key, _ := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	dir := t.TempDir()
	priv := filepath.Join(dir, "p.pem")
	pub := filepath.Join(dir, "pub.pem")
	der, _ := x509.MarshalPKCS8PrivateKey(key)
	_ = os.WriteFile(priv, pem.EncodeToMemory(&pem.Block{Type: "PRIVATE KEY", Bytes: der}), 0o600)
	pder, _ := x509.MarshalPKIXPublicKey(&key.PublicKey)
	_ = os.WriteFile(pub, pem.EncodeToMemory(&pem.Block{Type: "PUBLIC KEY", Bytes: pder}), 0o644)
	m, err := auth.NewManager(priv, pub, time.Hour)
	if err != nil {
		t.Fatal(err)
	}
	return m
}

// 驗證路由樹可成功建立(不會因 /users/me 與 /users/:user_id 衝突而 panic),
// 且 RBAC middleware 在無 token 時擋下受保護端點。
func TestRouterRegistersAndGuards(t *testing.T) {
	gin.SetMode(gin.TestMode)
	jm := testManager(t)
	r := NewRouter(&handler.Handler{}, jm)

	t.Run("healthz ok", func(t *testing.T) {
		w := httptest.NewRecorder()
		r.ServeHTTP(w, httptest.NewRequest(http.MethodGet, "/healthz", nil))
		if w.Code != http.StatusOK {
			t.Fatalf("healthz = %d", w.Code)
		}
	})

	t.Run("protected route without token -> 401", func(t *testing.T) {
		w := httptest.NewRecorder()
		r.ServeHTTP(w, httptest.NewRequest(http.MethodGet, "/api/users/me", nil))
		if w.Code != http.StatusUnauthorized {
			t.Fatalf("users/me without token = %d, want 401", w.Code)
		}
	})

	t.Run("admin route as no-auth -> 401", func(t *testing.T) {
		w := httptest.NewRecorder()
		r.ServeHTTP(w, httptest.NewRequest(http.MethodDelete, "/api/problems/p1", nil))
		if w.Code != http.StatusUnauthorized {
			t.Fatalf("delete problem = %d, want 401", w.Code)
		}
	})
}
