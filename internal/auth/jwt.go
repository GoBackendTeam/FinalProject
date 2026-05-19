package auth

import (
	"crypto/ecdsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/golang-jwt/jwt/v5"
)

// Manager 以 EC P-256(ES256)簽發/驗證 JWT,金鑰由 PRD 指定的
// openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:P-256 產生。
type Manager struct {
	priv *ecdsa.PrivateKey
	pub  *ecdsa.PublicKey
	ttl  time.Duration
}

type Claims struct {
	UserID   string     `json:"uid"`
	Username string     `json:"username"`
	Role     model.Role `json:"role"`
	jwt.RegisteredClaims
}

func NewManager(privPath, pubPath string, ttl time.Duration) (*Manager, error) {
	priv, err := loadPrivate(privPath)
	if err != nil {
		return nil, fmt.Errorf("load private key: %w", err)
	}
	pub, err := loadPublic(pubPath)
	if err != nil {
		return nil, fmt.Errorf("load public key: %w", err)
	}
	return &Manager{priv: priv, pub: pub, ttl: ttl}, nil
}

func loadPrivate(path string) (*ecdsa.PrivateKey, error) {
	raw, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	block, _ := pem.Decode(raw)
	if block == nil {
		return nil, errors.New("invalid PEM")
	}
	// openssl genpkey 產出的是 PKCS#8。
	if key, err := x509.ParsePKCS8PrivateKey(block.Bytes); err == nil {
		if ec, ok := key.(*ecdsa.PrivateKey); ok {
			return ec, nil
		}
		return nil, errors.New("private key is not EC")
	}
	return x509.ParseECPrivateKey(block.Bytes)
}

func loadPublic(path string) (*ecdsa.PublicKey, error) {
	raw, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	block, _ := pem.Decode(raw)
	if block == nil {
		return nil, errors.New("invalid PEM")
	}
	key, err := x509.ParsePKIXPublicKey(block.Bytes)
	if err != nil {
		return nil, err
	}
	ec, ok := key.(*ecdsa.PublicKey)
	if !ok {
		return nil, errors.New("public key is not EC")
	}
	return ec, nil
}

func (m *Manager) Sign(u *model.User) (string, error) {
	now := time.Now()
	c := Claims{
		UserID:   u.ID,
		Username: u.Username,
		Role:     u.Role,
		RegisteredClaims: jwt.RegisteredClaims{
			Subject:   u.ID,
			IssuedAt:  jwt.NewNumericDate(now),
			ExpiresAt: jwt.NewNumericDate(now.Add(m.ttl)),
		},
	}
	return jwt.NewWithClaims(jwt.SigningMethodES256, c).SignedString(m.priv)
}

func (m *Manager) Verify(token string) (*Claims, error) {
	c := &Claims{}
	_, err := jwt.ParseWithClaims(token, c, func(t *jwt.Token) (any, error) {
		if _, ok := t.Method.(*jwt.SigningMethodECDSA); !ok {
			return nil, fmt.Errorf("unexpected signing method: %v", t.Header["alg"])
		}
		return m.pub, nil
	})
	if err != nil {
		return nil, err
	}
	return c, nil
}
