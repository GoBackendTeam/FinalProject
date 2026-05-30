package api

import (
	"net/http"
	"os"

	"github.com/gin-gonic/gin"
)

// 互動式 API 文件:以 Scalar 渲染 OpenAPI 規格(比 Swagger UI 更現代、內建 API console)。
// 單一真相來源仍是 docs/openapi.yaml;此處只負責「提供瀏覽」。
//
//   - GET /docs              → Scalar HTML(從 CDN 載入 renderer,讀 /openapi.yaml)
//   - GET /openapi.yaml      → 直接吐出 OpenAPI 規格檔
//
// 若要完全離線,可把 Scalar 的 JS 改為自帶 static 檔;此處用 CDN 以零建置成本為先。
const scalarHTML = `<!doctype html>
<html>
  <head>
    <title>REGS API Reference</title>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
  </head>
  <body>
    <script id="api-reference" data-url="/openapi.yaml"></script>
    <script src="https://cdn.jsdelivr.net/npm/@scalar/api-reference"></script>
  </body>
</html>`

// registerDocs 掛上互動文件與規格檔路由。specPath 為 OpenAPI YAML 的路徑。
func registerDocs(r *gin.Engine, specPath string) {
	r.GET("/docs", func(c *gin.Context) {
		c.Data(http.StatusOK, "text/html; charset=utf-8", []byte(scalarHTML))
	})
	r.GET("/openapi.yaml", func(c *gin.Context) {
		data, err := os.ReadFile(specPath)
		if err != nil {
			c.JSON(http.StatusServiceUnavailable, gin.H{
				"error": "OpenAPI spec not found; set OPENAPI_SPEC_PATH or ship docs/openapi.yaml",
			})
			return
		}
		c.Data(http.StatusOK, "application/yaml; charset=utf-8", data)
	})
}
