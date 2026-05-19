package judge

import (
	"testing"

	"github.com/GoBackendTeam/FinalProject/internal/model"
)

func TestClassifyCase(t *testing.T) {
	cases := []struct {
		name      string
		exit      int
		timedOut  bool
		oom       bool
		want      model.Status
	}{
		{"accepted", 0, false, false, model.StatusAC},
		{"catch2 failures -> WA", 3, false, false, model.StatusWA},
		{"segfault -> RE", 139, false, false, model.StatusRE},
		{"abort -> RE", 134, false, false, model.StatusRE},
		{"timeout -> TLE", 137, true, false, model.StatusTLE},
		{"oom -> RE", 137, false, true, model.StatusRE},
	}
	for _, tc := range cases {
		if got := classifyCase(tc.exit, tc.timedOut, tc.oom); got != tc.want {
			t.Errorf("%s: classifyCase(%d,%v,%v)=%s want %s",
				tc.name, tc.exit, tc.timedOut, tc.oom, got, tc.want)
		}
	}
}

func TestAggregate(t *testing.T) {
	mk := func(ss ...model.Status) []model.CaseResult {
		var r []model.CaseResult
		for _, s := range ss {
			r = append(r, model.CaseResult{Status: s})
		}
		return r
	}
	tests := []struct {
		name string
		in   []model.CaseResult
		want model.Status
	}{
		{"all ac", mk(model.StatusAC, model.StatusAC), model.StatusAC},
		{"wa beats ac", mk(model.StatusAC, model.StatusWA), model.StatusWA},
		{"re worst", mk(model.StatusWA, model.StatusTLE, model.StatusRE), model.StatusRE},
		{"tle over wa", mk(model.StatusWA, model.StatusTLE), model.StatusTLE},
		{"empty -> IE", nil, model.StatusIE},
	}
	for _, tc := range tests {
		if got := aggregate(tc.in); got != tc.want {
			t.Errorf("%s: aggregate=%s want %s", tc.name, got, tc.want)
		}
	}
}
