package config

import "testing"

func TestPathForBind(t *testing.T) {
	appRoot := "/app/problems"
	hostRoot := "/host/problems"
	got := PathForBind("/app/problems/113final006", appRoot, hostRoot)
	want := "/host/problems/113final006"
	if got != want {
		t.Fatalf("PathForBind() = %q, want %q", got, want)
	}
}

func TestPathForBindSameRoot(t *testing.T) {
	root := "/Users/dev/FinalProject/problems"
	got := PathForBind(root+"/113final006", root, root)
	want := root + "/113final006"
	if got != want {
		t.Fatalf("PathForBind() = %q, want %q", got, want)
	}
}

func TestProblemDirForBind(t *testing.T) {
	c := &Config{
		ProblemsRoot:     "/app/problems",
		ProblemsRootHost: "/host/problems",
	}
	got := c.ProblemDirForBind("/app/problems/113final003")
	want := "/host/problems/113final003"
	if got != want {
		t.Fatalf("ProblemDirForBind() = %q, want %q", got, want)
	}
}
