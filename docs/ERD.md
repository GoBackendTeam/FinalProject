# 資料庫 ERD

```mermaid
erDiagram
    USER ||--o{ SUBMISSION : submits
    PROBLEM ||--o{ SUBMISSION : targets
    SUBMISSION ||--o{ CASE_RESULT : has

    USER {
        string id PK "uuid"
        string username UK
        string password_hash
        string role "user | admin"
        time   created_at
    }
    PROBLEM {
        string id PK "e.g. 113final006"
        string title
        text   description
        string problem_dir "題庫內路徑(含 root CMakeLists.txt)"
        string archive_path "admin 上傳的題目壓縮檔"
        string case_names "逗號分隔的 case 名"
        time   created_at
        time   updated_at
    }
    SUBMISSION {
        string id PK "operatorId, uuid"
        string user_id FK
        string problem_id FK
        string status "PENDING|RUNNING|AC|WA|CE|RE|TLE|SE|IE"
        string source_archive_path
        string workspace_dir
        string configure_log_path
        string compile_log_path
        string output_log_path
        text   message
        time   created_at
        time   started_at
        time   finished_at
    }
    CASE_RESULT {
        uint   id PK
        string submission_id FK
        string case_name
        string status
        int    exit_code
        int64  duration_ms
    }
```

## 說明

- **USER**:`role` 僅 `user` / `admin` 入庫;未帶 JWT 即視為 Guest(不入庫)。密碼以 bcrypt 雜湊。
- **PROBLEM**:`id` 即題目代號,亦為 CMake `project()` 名稱與執行檔前綴 `<project>-<case>`。
  題庫目錄含 root `CMakeLists.txt`、`cmake/AddJudge.cmake`、`spec/case*.h`。
- **SUBMISSION**:`id` 對應 PRD 的 `operatorId`。三段日誌以實體檔案儲存,DB 只存路徑。
  狀態機:`PENDING → RUNNING → {AC|WA|CE|RE|TLE|SE}`;系統錯誤為 `IE`。
- **CASE_RESULT**:每個提交對每個 case 一列;`OnDelete:CASCADE` 隨提交刪除。重跑前會清除舊列。
