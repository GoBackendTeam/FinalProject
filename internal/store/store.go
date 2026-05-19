package store

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/model"
	"golang.org/x/crypto/bcrypt"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

type Store struct {
	DB *gorm.DB
}

func Open(dsn string) (*Store, error) {
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{
		Logger: logger.Default.LogMode(logger.Warn),
	})
	if err != nil {
		return nil, fmt.Errorf("connect db: %w", err)
	}
	if err := db.AutoMigrate(&model.User{}, &model.Problem{}, &model.Submission{}, &model.CaseResult{}); err != nil {
		return nil, fmt.Errorf("automigrate: %w", err)
	}
	return &Store{DB: db}, nil
}

// SeedProblems 掃描題庫根目錄,把每個含 CMakeLists.txt + spec/ 的子目錄登錄為題目。
func (s *Store) SeedProblems(problemsRoot string) error {
	entries, err := os.ReadDir(problemsRoot)
	if err != nil {
		return err
	}
	for _, e := range entries {
		if !e.IsDir() {
			continue
		}
		dir := filepath.Join(problemsRoot, e.Name())
		if _, err := os.Stat(filepath.Join(dir, "CMakeLists.txt")); err != nil {
			continue
		}
		cases := discoverCases(dir)
		if len(cases) == 0 {
			continue
		}
		p := model.Problem{
			ID:         e.Name(),
			Title:      e.Name(),
			ProblemDir: dir,
			CaseNames:  strings.Join(cases, ","),
		}
		// 已存在則只更新題庫路徑與 case 清單,保留 admin 編輯過的標題/描述。
		var existing model.Problem
		if err := s.DB.First(&existing, "id = ?", p.ID).Error; err != nil {
			if errors.Is(err, gorm.ErrRecordNotFound) {
				if err := s.DB.Create(&p).Error; err != nil {
					return err
				}
				continue
			}
			return err
		}
		existing.ProblemDir = dir
		existing.CaseNames = strings.Join(cases, ",")
		if err := s.DB.Save(&existing).Error; err != nil {
			return err
		}
	}
	return nil
}

func discoverCases(problemDir string) []string {
	specDir := filepath.Join(problemDir, "spec")
	entries, err := os.ReadDir(specDir)
	if err != nil {
		return nil
	}
	var cases []string
	for _, e := range entries {
		name := e.Name()
		if e.IsDir() || !strings.HasSuffix(name, ".h") {
			continue
		}
		cases = append(cases, strings.TrimSuffix(name, ".h"))
	}
	return cases
}

// EnsureAdmin 在系統首次啟動時建立預設 admin 帳號(若尚不存在)。
func (s *Store) EnsureAdmin(username, password string) error {
	var n int64
	s.DB.Model(&model.User{}).Where("role = ?", model.RoleAdmin).Count(&n)
	if n > 0 {
		return nil
	}
	hash, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		return err
	}
	return s.DB.Create(&model.User{
		ID:           newID(),
		Username:     username,
		PasswordHash: string(hash),
		Role:         model.RoleAdmin,
		CreatedAt:    time.Now(),
	}).Error
}
