# REGS — 線上自動評測系統(CS3060701 期末專案)

Go 後端,接收學生上傳的 C++ 專案壓縮檔,在隔離容器內以 **CMake + Ninja + Clang** 自動編譯,
逐 case 在**斷網容器**執行 Catch2 判題,回報 `AC / WA / CE / RE / TLE / SE`。

## 架構

```
HTTP(Gin) ──► Service ──► Judge Engine ──► Docker Engine API
  │                          │  Job Queue + Semaphore
  │                          │
RBAC(JWT ES256/P-256)        ├─ Compile 容器 (network=regs-build) cmake -G Ninja → cmake --build
PostgreSQL(GORM)             └─ Exec 容器 (--network none) 跑 judge 執行檔 + timeout
```

判題狀態判定:

| 階段 | 指令 | 失敗 → |
|---|---|---|
| Configure | `cmake -S /problem -B /build -G Ninja -D SOURCE_ROOT=/src` | **SE** |
| Build | `cmake --build /build --verbose` | **CE** |
| Execute(逐 case,斷網) | `/build/<project>-<case>` | exit 0 → **AC**;1‑127 → **WA**;≥128/訊號 → **RE**;逾時 → **TLE** |

三段輸出分別落地為 `configure.log` / `compile.log` / `output.log`,可由 API 查詢。

## 先決條件

- Go 1.25+
- Docker Desktop(**判題需要 daemon 運行中**)
- `openssl`(產生 JWT 金鑰)
- 評測映像 `yhlib/cs3060701`(首次啟動會自動 `docker pull`)

> macOS 注意:本服務預設「直接跑在主機」(`go run`),judge 容器的 bind mount
> 路徑與服務看到的一致,最單純。`./workspace` 在使用者家目錄下,Docker Desktop 預設可掛載。

## 快速開始

```bash
make keys          # 產生 keys/private.pem, keys/public.pem (EC P-256)
cp .env.example .env
make db            # 用 docker compose 起 PostgreSQL
make run           # 啟動 API(預設 :8080),自動 seed problems/ 與建立 admin 帳號
```

預設管理者帳號:`admin / admin`(可用 `ADMIN_USERNAME` / `ADMIN_PASSWORD` 覆蓋)。

## 端到端操作範例

```bash
# 1) 註冊 + 登入,取得 JWT
curl -s localhost:8080/api/users/register -d '{"username":"alice","password":"pass"}' -H 'Content-Type: application/json'
TOKEN=$(curl -s localhost:8080/api/users/login -d '{"username":"alice","password":"pass"}' -H 'Content-Type: application/json' | jq -r .token)

# 2) 看題目(已從 problems/ 自動 seed)
curl -s localhost:8080/api/problems | jq

# 3) 打包某題的 solution 當作「學生提交」(壓縮檔不含前導資料夾)
cd problems/113final006/solution && zip -r /tmp/sol.zip . && cd -

# 4) 提交 → 立即拿到 operatorId
OP=$(curl -s -H "Authorization: Bearer $TOKEN" -F problem_id=113final006 -F file=@/tmp/sol.zip \
     localhost:8080/api/submissions | jq -r .operatorId)

# 5) 輪詢結果
curl -s -H "Authorization: Bearer $TOKEN" localhost:8080/api/submissions/$OP | jq

# 6) 查三段日誌
curl -s -H "Authorization: Bearer $TOKEN" localhost:8080/api/submissions/$OP/logs/compile | jq -r .log

# 7) 重跑該 Job
curl -s -X POST -H "Authorization: Bearer $TOKEN" localhost:8080/api/submissions/$OP/rerun
```

## API

完整規格見 [`docs/openapi.yaml`](docs/openapi.yaml);資料庫關係見 [`docs/ERD.md`](docs/ERD.md)。

伺服器啟動後可直接瀏覽互動式文件(規格內嵌於 binary,免額外部署):

| 路徑 | 說明 |
|---|---|
| `GET /docs` | Scalar 互動式文件(內建 API console) |
| `GET /openapi.yaml` | 原始 OpenAPI 3.0 規格 |

> UI 的 renderer 由 CDN 載入,瀏覽器需可連外網;`docs/CLOUD_READINESS.md` 為雲端部署評估。

權限:`Guest < User < Admin`。標 *(User/Admin)* 的端點必須帶 `Authorization: Bearer <JWT>`。

| 方法 | 路徑 | 權限 |
|---|---|---|
| POST | `/api/users/register` | Guest |
| POST | `/api/users/login` | Guest |
| POST | `/api/users/logout` | User |
| GET | `/api/users/me` | User |
| GET | `/api/users/{user_id}/submissions` | Guest |
| GET | `/api/problems` | Guest |
| GET | `/api/problems/{problem_id}` | Guest |
| PUT | `/api/problems` | Admin |
| DELETE | `/api/problems/{problem_id}` | Admin |
| GET | `/api/problems/{problem_id}/testcases` | Admin |
| POST | `/api/submissions` | User |
| GET | `/api/submissions` | User |
| GET | `/api/submissions/{operatorId}` | User |
| GET | `/api/submissions/{operatorId}/source` | User |
| GET | `/api/submissions/{operatorId}/logs/{stage}` | User |
| POST | `/api/submissions/{operatorId}/rerun` | User |
| GET | `/api/stats/problems/{problem_id}` | Guest |
| GET | `/api/stats/users/{user_id}` | Guest |

`stage` ∈ `configure | compile | output`。

## 設定(環境變數)

見 [`.env.example`](.env.example)。重點:

- `JUDGE_MAX_CONCURRENCY` — Semaphore 上限,同時執行中的評測數。
- `JUDGE_CASE_TIMEOUT` / `JUDGE_BUILD_TIMEOUT` — TLE 與編譯逾時(秒)。
- `JUDGE_WORKSPACE_HOST` — 若把 app 也容器化(DooD),需指向**主機**真實路徑。

## 目錄

```
cmd/server         進入點(裝配 + 優雅關閉)
internal/config    環境設定
internal/model     GORM 資料模型
internal/store     DB 連線 / automigrate / seed / 預設 admin
internal/auth      JWT(ES256, EC P-256)
internal/archive   安全解壓(zip/tar/tgz,zip-slip 防護,前導夾攤平)
internal/judge     評測引擎:engine(Queue+Semaphore)/pipeline/docker/status
internal/api       Gin router + RBAC middleware + handlers
problems/          題庫(113final00X,啟動時自動登錄)
docs/              OpenAPI 3.0 / ERD
```

## 測試

```bash
go test ./...   # 解壓防護、狀態判定、JWT 簽驗、路由/RBAC smoke
```

判題完整流程需 Docker daemon;單元測試不需。
