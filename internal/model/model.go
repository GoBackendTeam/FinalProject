package model

import "time"

// Role 為 RBAC 角色。Guest 不入庫(無 token 即 Guest),DB 內只有 user/admin。
type Role string

const (
	RoleGuest Role = "guest"
	RoleUser  Role = "user"
	RoleAdmin Role = "admin"
)

// Status 為提交/單一 case 的評測狀態。
type Status string

const (
	StatusPending Status = "PENDING" // 已入列,尚未開始
	StatusRunning Status = "RUNNING" // 評測進行中
	StatusAC      Status = "AC"      // Accepted
	StatusWA      Status = "WA"      // Wrong Answer
	StatusCE      Status = "CE"      // Compilation Error
	StatusRE      Status = "RE"      // Runtime Error
	StatusTLE     Status = "TLE"     // Time Limit Exceeded
	StatusSE      Status = "SE"      // Setup Error(CMake configure 失敗)
	StatusIE      Status = "IE"      // Internal Error(系統內部錯誤)
)

type User struct {
	ID           string    `gorm:"primaryKey;size:36" json:"id"`
	Username     string    `gorm:"uniqueIndex;size:64;not null" json:"username"`
	PasswordHash string    `gorm:"not null" json:"-"`
	Role         Role      `gorm:"size:16;not null;default:user" json:"role"`
	CreatedAt    time.Time `json:"created_at"`
}

type Problem struct {
	ID          string    `gorm:"primaryKey;size:64" json:"id"` // e.g. "113final006"
	Title       string    `gorm:"size:255" json:"title"`
	Description string    `gorm:"type:text" json:"description"`
	// ProblemDir 為題庫內該題的目錄(含 root CMakeLists.txt 與 spec/)。
	ProblemDir string `gorm:"size:512" json:"-"`
	// ArchivePath 為 admin 上傳的題目壓縮檔(testcases 下載用)。
	ArchivePath string    `gorm:"size:512" json:"-"`
	CaseNames   string    `gorm:"size:512" json:"-"` // 逗號分隔的 case 名
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type Submission struct {
	// ID 即 PRD 的 operatorId。
	ID        string `gorm:"primaryKey;size:36" json:"operatorId"`
	UserID    string `gorm:"index;size:36;not null" json:"user_id"`
	ProblemID string `gorm:"index;size:64;not null" json:"problem_id"`

	Status Status `gorm:"size:16;not null;index" json:"status"`

	// SourceArchivePath 為使用者上傳的原始壓縮檔(供 /source 下載)。
	SourceArchivePath string `gorm:"size:512" json:"-"`
	WorkspaceDir      string `gorm:"size:512" json:"-"`

	ConfigureLogPath string `gorm:"size:512" json:"-"`
	CompileLogPath   string `gorm:"size:512" json:"-"`
	OutputLogPath    string `gorm:"size:512" json:"-"`

	Message    string     `gorm:"type:text" json:"message"`
	CreatedAt  time.Time  `json:"created_at"`
	StartedAt  *time.Time `json:"started_at,omitempty"`
	FinishedAt *time.Time `json:"finished_at,omitempty"`

	CaseResults []CaseResult `gorm:"foreignKey:SubmissionID;constraint:OnDelete:CASCADE" json:"case_results,omitempty"`
}

type CaseResult struct {
	ID           uint   `gorm:"primaryKey;autoIncrement" json:"-"`
	SubmissionID string `gorm:"index;size:36;not null" json:"-"`
	CaseName     string `gorm:"size:64;not null" json:"case_name"`
	Status       Status `gorm:"size:16;not null" json:"status"`
	ExitCode     int    `json:"exit_code"`
	DurationMs   int64  `json:"duration_ms"`
}
