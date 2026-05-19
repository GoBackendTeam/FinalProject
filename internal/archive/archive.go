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

const maxFileSize = 64 << 20 // 單檔上限 64MB,避免解壓炸彈

// Extract 將 archivePath 解壓到 destDir(必須已存在且為空)。
// 解壓後若內容全部位於單一頂層資料夾下,會自動把該層攤平。
func Extract(archivePath, destDir string) error {
	lower := strings.ToLower(archivePath)
	var err error
	switch {
	case strings.HasSuffix(lower, ".zip"):
		err = extractZip(archivePath, destDir)
	case strings.HasSuffix(lower, ".tar"):
		err = extractTar(archivePath, destDir, false)
	case strings.HasSuffix(lower, ".tar.gz"), strings.HasSuffix(lower, ".tgz"):
		err = extractTar(archivePath, destDir, true)
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

func extractZip(src, dest string) error {
	r, err := zip.OpenReader(src)
	if err != nil {
		return err
	}
	defer r.Close()
	for _, f := range r.File {
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
		err = writeFile(target, rc)
		rc.Close()
		if err != nil {
			return err
		}
	}
	return nil
}

func extractTar(src, dest string, gzipped bool) error {
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
			if err := writeFile(target, tr); err != nil {
				return err
			}
		}
	}
	return nil
}

func writeFile(target string, r io.Reader) error {
	out, err := os.OpenFile(target, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0o644)
	if err != nil {
		return err
	}
	defer out.Close()
	if _, err := io.CopyN(out, r, maxFileSize); err != nil && !errors.Is(err, io.EOF) {
		return err
	}
	return nil
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
