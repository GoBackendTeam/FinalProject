package judge

import (
	"context"
	"log"
	"sync"
	"time"

	"github.com/GoBackendTeam/FinalProject/internal/config"
	"github.com/GoBackendTeam/FinalProject/internal/model"
	"github.com/GoBackendTeam/FinalProject/internal/store"
)

// Engine 為非同步評測引擎:Job Queue + Semaphore 併發控制。
//
//   - jobs:無界感的待評測佇列(收到提交立即入列並回 operatorId)
//   - sem :號誌,限制「同時執行中」的評測數 = JudgeMaxConcurrency
//
// 任意提交都可透過 Rerun 重新入列(對應 rubric「可重新執行某個 Job」)。
type Engine struct {
	cfg    *config.Config
	st     *store.Store
	pl     *pipeline
	docker *dockerRunner

	jobs   chan string
	sem    chan struct{}
	wg     sync.WaitGroup
	ctx    context.Context
	cancel context.CancelFunc
}

func NewEngine(cfg *config.Config, st *store.Store) (*Engine, error) {
	d, err := newDockerRunner(cfg.JudgePlatform)
	if err != nil {
		return nil, err
	}
	e := &Engine{
		cfg:    cfg,
		st:     st,
		docker: d,
		pl:     &pipeline{cfg: cfg, st: st, docker: d},
		jobs:   make(chan string, 4096),
		sem:    make(chan struct{}, cfg.JudgeMaxConcurrency),
	}
	return e, nil
}

// Start 啟動派工器。Docker 不可用時只記錄警告,不阻擋服務啟動。
func (e *Engine) Start(ctx context.Context) {
	e.ctx, e.cancel = context.WithCancel(ctx)

	pingCtx, cancel := context.WithTimeout(ctx, 5*time.Second)
	defer cancel()
	if err := e.docker.ping(pingCtx); err != nil {
		log.Printf("[judge] WARNING: docker daemon 無法連線(%v);提交會入列但無法評測,請啟動 Docker 後重跑 job", err)
	} else {
		if err := e.docker.ensureNetwork(ctx, e.cfg.JudgeBuildNetwork); err != nil {
			log.Printf("[judge] WARNING: 無法建立 build network %q: %v", e.cfg.JudgeBuildNetwork, err)
		}
		go func() {
			pullCtx, cancel := context.WithTimeout(context.Background(), 10*time.Minute)
			defer cancel()
			if err := e.docker.ensureImage(pullCtx, e.cfg.JudgeImage); err != nil {
				log.Printf("[judge] WARNING: 評測映像 %q 不可用(%v);請先 docker pull", e.cfg.JudgeImage, err)
			} else {
				log.Printf("[judge] 評測映像就緒: %s", e.cfg.JudgeImage)
			}
		}()
	}

	go e.dispatch()
	log.Printf("[judge] engine started (max concurrency=%d, image=%s)", e.cfg.JudgeMaxConcurrency, e.cfg.JudgeImage)
}

func (e *Engine) dispatch() {
	for {
		select {
		case <-e.ctx.Done():
			return
		case id := <-e.jobs:
			select {
			case e.sem <- struct{}{}: // 取得號誌槽(滿則阻塞 → 維持 Pending)
			case <-e.ctx.Done():
				return
			}
			e.wg.Add(1)
			go func(subID string) {
				defer e.wg.Done()
				defer func() { <-e.sem }()
				e.process(subID)
			}(id)
		}
	}
}

// Stop 停止派工並等待進行中的評測結束。
func (e *Engine) Stop() {
	if e.cancel != nil {
		e.cancel()
	}
	e.wg.Wait()
}

// Enqueue 將提交加入評測佇列(非阻塞,不會卡住 HTTP handler)。
func (e *Engine) Enqueue(submissionID string) {
	go func() {
		select {
		case e.jobs <- submissionID:
		case <-e.ctx.Done():
		}
	}()
}

// Rerun 重設提交狀態並重新入列。
func (e *Engine) Rerun(submissionID string) error {
	if err := e.st.DB.Model(&model.Submission{}).
		Where("id = ?", submissionID).
		Updates(map[string]any{
			"status":      model.StatusPending,
			"message":     "",
			"started_at":  nil,
			"finished_at": nil,
		}).Error; err != nil {
		return err
	}
	e.st.DB.Where("submission_id = ?", submissionID).Delete(&model.CaseResult{})
	e.Enqueue(submissionID)
	return nil
}

func (e *Engine) process(id string) {
	defer func() {
		if r := recover(); r != nil {
			log.Printf("[judge] panic on submission %s: %v", id, r)
			e.st.DB.Model(&model.Submission{}).Where("id = ?", id).
				Updates(map[string]any{"status": model.StatusIE, "message": "internal panic"})
		}
	}()

	var sub model.Submission
	if err := e.st.DB.First(&sub, "id = ?", id).Error; err != nil {
		log.Printf("[judge] submission %s not found: %v", id, err)
		return
	}
	var prob model.Problem
	if err := e.st.DB.First(&prob, "id = ?", sub.ProblemID).Error; err != nil {
		e.pl.fail(&sub, model.StatusIE, "problem not found: "+sub.ProblemID)
		return
	}

	// 單一提交的整體上限:設定 + 編譯 + 全部 case,加緩衝。
	budget := e.cfg.JudgeBuildTimeout*2 +
		e.cfg.JudgeCaseTimeout*time.Duration(len(splitCases(prob.CaseNames))+1) +
		60*time.Second
	jobCtx, cancel := context.WithTimeout(context.Background(), budget)
	defer cancel()

	if err := e.pl.Run(jobCtx, &sub, &prob); err != nil {
		log.Printf("[judge] submission %s pipeline error: %v", id, err)
	}
}
