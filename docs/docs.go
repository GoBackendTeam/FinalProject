// Package docs 將 API 規格與互動式文件頁面打包進 binary,
// 讓 /openapi.yaml、/docs 不依賴執行時的工作目錄。
package docs

import _ "embed"

//go:embed openapi.yaml
var OpenAPIYAML []byte

//go:embed scalar.html
var ScalarHTML []byte
