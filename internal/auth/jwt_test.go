package auth

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"encoding/pem"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/model"
)

// writeTestKeys 產生 P-256 金鑰並以 PKCS#8 / PKIX PEM 寫出(等同 openssl genpkey)。
func writeTestKeys(t *testing.T) (string, string) {
	t.Helper()
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		t.Fatal(err)
	}
	dir := t.TempDir()
	privP := filepath.Join(dir, "private.pem")
	pubP := filepath.Join(dir, "public.pem")

	der, _ := x509.MarshalPKCS8PrivateKey(key)
	_ = os.WriteFile(privP, pem.EncodeToMemory(&pem.Block{Type: "PRIVATE KEY", Bytes: der}), 0o600)

	pder, _ := x509.MarshalPKIXPublicKey(&key.PublicKey)
	_ = os.WriteFile(pubP, pem.EncodeToMemory(&pem.Block{Type: "PUBLIC KEY", Bytes: pder}), 0o644)
	return privP, pubP
}

func TestSignVerifyRoundTrip(t *testing.T) {
	priv, pub := writeTestKeys(t)
	m, err := NewManager(priv, pub, time.Hour)
	if err != nil {
		t.Fatalf("NewManager: %v", err)
	}
	u := &model.User{ID: "u1", Username: "alice", Role: model.RoleAdmin}
	tok, err := m.Sign(u)
	if err != nil {
		t.Fatalf("Sign: %v", err)
	}
	cl, err := m.Verify(tok)
	if err != nil {
		t.Fatalf("Verify: %v", err)
	}
	if cl.UserID != "u1" || cl.Role != model.RoleAdmin {
		t.Fatalf("claims mismatch: %+v", cl)
	}
	if _, err := m.Verify(tok + "tamper"); err == nil {
		t.Fatal("expected tampered token to fail verification")
	}
}
