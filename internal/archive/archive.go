// Package archive 負責安全解壓使用者上傳的專案壓縮檔。
// 支援 .zip 與 gzip 系列(.tar / .tar.gz / .tgz),含 zip-slip 防護,
// 並在偵測到單一前導資料夾時自動攤平(對應 PRD「解壓後不應包含根資料夾」)。
package archive

import (
	"archive/tar"
	"archive/zip"
	"compress/gzip"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
)

var ErrUnsupported = errors.New("unsupported archive format")

const (
	maxFileSize  = 64 << 20  // 單檔解壓上限 64MB
	maxTotalSize = 256 << 20 // 解壓後總量上限 256MB(擋 zip bomb)
	maxEntries   = 4000      // 條目數上限(擋大量小檔)
)

// guard 追蹤解壓累計量,超限即中止(對抗 zip bomb / 巨量條目)。
type guard struct {
	totalBytes int64
	entries    int
}

func (g *guard) addEntry() error {
	g.entries++
	if g.entries > maxEntries {
		return fmt.Errorf("archive has too many entries (> %d)", maxEntries)
	}
	return nil
}

func (g *guard) addBytes(n int64) error {
	g.totalBytes += n
	if g.totalBytes > maxTotalSize {
		return fmt.Errorf("archive uncompressed size exceeds %d bytes", maxTotalSize)
	}
	return nil
}

// Extract 將 archivePath 解壓到 destDir(必須已存在且為空)。
// 解壓後若內容全部位於單一頂層資料夾下,會自動把該層攤平。
func Extract(archivePath, destDir string) error {
	lower := strings.ToLower(archivePath)
	g := &guard{}
	var err error
	switch {
	case strings.HasSuffix(lower, ".zip"):
		err = extractZip(archivePath, destDir, g)
	case strings.HasSuffix(lower, ".tar"):
		err = extractTar(archivePath, destDir, false, g)
	case strings.HasSuffix(lower, ".tar.gz"), strings.HasSuffix(lower, ".tgz"):
		err = extractTar(archivePath, destDir, true, g)
	default:
		return ErrUnsupported
	}
	if err != nil {
		return err
	}
	return flattenSingleRoot(destDir)
}

// safeJoin 防止 zip-slip:確保 name 解析後仍位於 dest 之內。
func safeJoin(dest, name string) (string, error) {
	target := filepath.Join(dest, name)
	cleanDest := filepath.Clean(dest) + string(os.PathSeparator)
	if !strings.HasPrefix(filepath.Clean(target)+string(os.PathSeparator), cleanDest) {
		return "", fmt.Errorf("illegal path in archive: %q", name)
	}
	return target, nil
}

func extractZip(src, dest string, g *guard) error {
	r, err := zip.OpenReader(src)
	if err != nil {
		return err
	}
	defer r.Close()
	for _, f := range r.File {
		if err := g.addEntry(); err != nil {
			return err
		}
		target, err := safeJoin(dest, f.Name)
		if err != nil {
			return err
		}
		if f.FileInfo().IsDir() {
			if err := os.MkdirAll(target, 0o755); err != nil {
				return err
			}
			continue
		}
		if err := os.MkdirAll(filepath.Dir(target), 0o755); err != nil {
			return err
		}
		rc, err := f.Open()
		if err != nil {
			return err
		}
		err = writeFile(target, rc, g)
		rc.Close()
		if err != nil {
			return err
		}
	}
	return nil
}

func extractTar(src, dest string, gzipped bool, g *guard) error {
	f, err := os.Open(src)
	if err != nil {
		return err
	}
	defer f.Close()

	var r io.Reader = f
	if gzipped {
		gz, err := gzip.NewReader(f)
		if err != nil {
			return err
		}
		defer gz.Close()
		r = gz
	}

	tr := tar.NewReader(r)
	for {
		hdr, err := tr.Next()
		if errors.Is(err, io.EOF) {
			break
		}
		if err != nil {
			return err
		}
		if err := g.addEntry(); err != nil {
			return err
		}
		target, err := safeJoin(dest, hdr.Name)
		if err != nil {
			return err
		}
		switch hdr.Typeflag {
		case tar.TypeDir:
			if err := os.MkdirAll(target, 0o755); err != nil {
				return err
			}
		case tar.TypeReg:
			if err := os.MkdirAll(filepath.Dir(target), 0o755); err != nil {
				return err
			}
			if err := writeFile(target, tr, g); err != nil {
				return err
			}
		}
	}
	return nil
}

func writeFile(target string, r io.Reader, g *guard) error {
	out, err := os.OpenFile(target, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0o644)
	if err != nil {
		return err
	}
	defer out.Close()
	// 多讀 1 byte:若還有資料代表超過單檔上限 → 明確報錯而非靜默截斷。
	n, err := io.Copy(out, io.LimitReader(r, maxFileSize+1))
	if err != nil {
		return err
	}
	if n > maxFileSize {
		return fmt.Errorf("file %q exceeds per-file limit of %d bytes", filepath.Base(target), maxFileSize)
	}
	return g.addBytes(n)
}

// flattenSingleRoot:若 destDir 下只有一個項目且為目錄,將其內容上提一層。
func flattenSingleRoot(destDir string) error {
	entries, err := os.ReadDir(destDir)
	if err != nil {
		return err
	}
	// 忽略 macOS zip 常見的 __MACOSX。
	var real []os.DirEntry
	for _, e := range entries {
		if e.Name() == "__MACOSX" || e.Name() == ".DS_Store" {
			_ = os.RemoveAll(filepath.Join(destDir, e.Name()))
			continue
		}
		real = append(real, e)
	}
	if len(real) != 1 || !real[0].IsDir() {
		return nil
	}
	root := filepath.Join(destDir, real[0].Name())
	inner, err := os.ReadDir(root)
	if err != nil {
		return err
	}
	for _, e := range inner {
		from := filepath.Join(root, e.Name())
		to := filepath.Join(destDir, e.Name())
		if err := os.Rename(from, to); err != nil {
			return err
		}
	}
	return os.Remove(root)
}

// HasCMakeLists 檢查目錄根是否含 CMakeLists.txt(題目壓縮檔上傳時校驗用)。
func HasCMakeLists(dir string) bool {
	_, err := os.Stat(filepath.Join(dir, "CMakeLists.txt"))
	return err == nil
}
