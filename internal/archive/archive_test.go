package archive

import (
	"archive/zip"
	"os"
	"path/filepath"
	"testing"
)

func makeZip(t *testing.T, entries map[string]string) string {
	t.Helper()
	p := filepath.Join(t.TempDir(), "a.zip")
	f, err := os.Create(p)
	if err != nil {
		t.Fatal(err)
	}
	defer f.Close()
	zw := zip.NewWriter(f)
	for name, body := range entries {
		w, err := zw.Create(name)
		if err != nil {
			t.Fatal(err)
		}
		_, _ = w.Write([]byte(body))
	}
	if err := zw.Close(); err != nil {
		t.Fatal(err)
	}
	return p
}

func TestExtractFlattensSingleRoot(t *testing.T) {
	src := makeZip(t, map[string]string{
		"proj/CMakeLists.txt": "cmake_minimum_required(VERSION 3.20)\n",
		"proj/main.cpp":       "int main(){}\n",
	})
	dest := t.TempDir()
	if err := Extract(src, dest); err != nil {
		t.Fatalf("extract: %v", err)
	}
	if !HasCMakeLists(dest) {
		t.Fatalf("expected CMakeLists.txt at dest root after flatten; got %v", lsDir(dest))
	}
	if _, err := os.Stat(filepath.Join(dest, "main.cpp")); err != nil {
		t.Fatalf("expected main.cpp at root: %v", err)
	}
}

func TestExtractRejectsZipSlip(t *testing.T) {
	src := makeZip(t, map[string]string{
		"../evil.txt": "pwned",
	})
	dest := t.TempDir()
	if err := Extract(src, dest); err == nil {
		t.Fatal("expected zip-slip to be rejected")
	}
}

func TestExtractUnsupportedFormat(t *testing.T) {
	p := filepath.Join(t.TempDir(), "x.rar")
	_ = os.WriteFile(p, []byte("x"), 0o644)
	if err := Extract(p, t.TempDir()); err != ErrUnsupported {
		t.Fatalf("expected ErrUnsupported, got %v", err)
	}
}

func lsDir(d string) []string {
	es, _ := os.ReadDir(d)
	var out []string
	for _, e := range es {
		out = append(out, e.Name())
	}
	return out
}
