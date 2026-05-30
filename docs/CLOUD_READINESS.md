# REGS — Cloud Deployment Readiness Assessment

> Scope: evaluates the current architecture (`internal/…`, `cmd/server`, `docker-compose.yml`,
> `Dockerfile`) against what a cloud deployment requires. Verdict is per-concern with a
> prioritized checklist at the end.

## TL;DR verdict

**Ready for a single-VM cloud deployment today (low effort). Not ready for a managed
serverless/auto-scaling deployment without architectural changes.**

The blocker is the **judge execution model**: the app talks to a host Docker daemon and
shares a **bind-mounted filesystem** with the judge containers (`JUDGE_WORKSPACE` ↔
`JUDGE_WORKSPACE_HOST`). That couples the API process to a Docker host and to local disk —
which is fine on one VM, but breaks the moment you want stateless, horizontally-scaled
replicas on Cloud Run / App Engine / Fargate / K8s without extra plumbing.

The application layer itself is in good shape: 12-factor config, JWT auth, RBAC, graceful
shutdown, crash recovery, container hardening, upload guards. The gaps are operational
(secrets, TLS, observability, durable queue, shared storage) rather than code-quality.

---

## What's already cloud-friendly ✅

| Area | Evidence | Note |
|---|---|---|
| 12-factor config | `internal/config` — all settings from env | Clean; maps to ConfigMaps/Secrets |
| Stateless auth | JWT ES256, no server session | Scales horizontally (auth-wise) |
| Graceful shutdown | `cmd/server/main.go` — SIGINT/TERM, 15s drain, `eng.Stop()` | Plays well with rolling deploys / SIGTERM |
| Crash recovery | `engine.recoverIncomplete()` re-enqueues PENDING/RUNNING on boot | No permanently-stuck jobs |
| Health endpoint | `GET /healthz` | LB/K8s liveness probe ready |
| Container build | Multi-stage `Dockerfile`, `CGO_ENABLED=0` static binary | Small, portable image |
| DB via managed-friendly driver | GORM + Postgres (pgx) | Drop-in RDS / Cloud SQL / Neon |
| Workload hardening | exec container: `--network none`, read-only rootfs, `CapDrop ALL`, no-new-privileges, mem/CPU/pids limits, tmpfs | Strong sandbox baseline |
| Upload abuse guards | 100MB cap, zip-slip + zip-bomb protection (`internal/archive`) | |
| Backpressure | semaphore-bounded concurrency, bounded job channel | |

---

## Blockers & gaps 🚧

### P0 — Must fix before any internet-facing deployment

1. **Default admin credentials.** `EnsureAdmin("admin","admin")`. Set `ADMIN_USERNAME` /
   `ADMIN_PASSWORD` from a secret, or disable auto-create in prod.
2. **No TLS.** Server listens plain HTTP on `:8080`; JWTs and passwords go in the clear.
   Terminate TLS at an ingress/LB (Caddy, nginx, ALB, Cloud Run built-in) — do **not** ship
   the Go server to the public internet directly.
3. **Secrets in env/files.** JWT private key is a PEM on local disk; DB password is inline in
   `DATABASE_URL` with `sslmode=disable`. Move to a secret manager (AWS Secrets Manager / GCP
   Secret Manager / K8s Secrets) and require `sslmode=require` for the DB.
4. **`GIN_MODE=debug` default.** Set `GIN_MODE=release` in prod (verbose debug logging + stack
   exposure otherwise).
5. **Untrusted-code blast radius via Docker socket.** Judge containers run on the host daemon;
   access to that socket is root-equivalent on the node. For running *arbitrary student C++*
   in a shared cloud env, add a stronger sandbox boundary: gVisor (`runsc`), Kata Containers,
   or Firecracker microVMs, and run judge workers on **dedicated, isolated nodes**.

### P1 — Required for horizontal scaling / multi-replica

6. **Local-disk statefulness.** Submissions' source archives, build trees, and the three log
   files live under `JUDGE_WORKSPACE` on local disk; `/source` and `/logs/{stage}` read them
   back from that FS. Two API replicas can't see each other's files. Options:
   - **Single node** (keep as-is), or
   - move artifacts to **object storage** (S3/GCS) and stream from there, or
   - mount a **shared RWX volume** (EFS / Filestore / NFS PVC) across replicas.
7. **In-process job queue.** The queue is a Go channel inside the API process
   (`engine.jobs`). It is not durable beyond `recoverIncomplete` and doesn't coordinate across
   instances — scaling the API to N replicas would mean N independent dispatchers fighting over
   the same DB rows. To scale, externalize the queue (SQS / Pub-Sub / **RabbitMQ** / Redis
   Streams) and split **stateless API** from **stateful judge workers**.
   *(You already operate a RabbitMQ broker — see `ref_xerno_mq_broker` — a natural fit.)*
8. **Bind-mount path coupling.** `JUDGE_WORKSPACE_HOST` only works when the API and the judge
   containers share a host filesystem (DooD on one VM). This is incompatible with Cloud Run /
   App Engine / Fargate (no Docker socket, no host mounts). A K8s deployment needs either a
   privileged DinD sidecar + shared `emptyDir`, or a dedicated runner service.
9. **Schema migrations via `AutoMigrate` on boot.** Fine for a single instance; risky with
   concurrent replicas (migration races) and for controlled prod schema changes. Move to
   versioned migrations (golang-migrate / Atlas) run as a separate job/init container.

### P2 — Operational hardening (should-have)

10. **Observability.** Only stdlib `log` to stdout (captured by cloud logging — OK as a floor).
    Add: structured/JSON logs, request IDs, Prometheus metrics (queue depth, judge duration,
    verdict counts), and OpenTelemetry traces (the deps are already pulled in transitively).
11. **Readiness vs liveness.** `/healthz` always returns ok; it doesn't check DB or Docker
    reachability. Add a `/readyz` that pings both so LBs don't route to a broken instance.
12. **Rate limiting / quotas.** No throttling on `register` / `login` (credential stuffing) or
    `submissions` (compute-DoS — each submit spawns containers). Add per-IP / per-user limits.
13. **CORS.** Not configured; required if a browser SPA on another origin will call the API.
14. **Platform/arch.** `yhlib/cs3060701` is **amd64-only**. Run judge workers on amd64 nodes;
    avoid Graviton/arm node pools (QEMU emulation is slow and fragile).
15. **CI/CD.** No pipeline present. Add build + `go test` + image push + deploy.

---

## Recommended target architectures

### Option A — Single VM (lowest effort, matches current design) — recommended first step
```
Internet → Caddy/nginx (TLS) → REGS app container ─┬─ Docker socket (DooD) → judge containers
                                                    └─ shared ./workspace bind mount
                                            managed Postgres (RDS / Cloud SQL / Neon)
```
- `docker-compose.yml` already has a commented `app` service with the DooD wiring
  (`/var/run/docker.sock` + `JUDGE_WORKSPACE_HOST=${PWD}/workspace`). Uncomment, put Caddy in
  front for TLS, point `DATABASE_URL` at managed Postgres, inject secrets.
- Good enough for a course project, demo, or small cohort. Single point of failure; vertical
  scaling only.

### Option B — Scalable split (when you outgrow one VM)
```
Internet → LB/TLS → [API replicas: stateless, Cloud Run/ECS/K8s]
                          │  enqueue
                          ▼
                 durable queue (RabbitMQ / SQS / Pub-Sub)
                          │  consume
                          ▼
        [judge worker pool: VMs/K8s nodes w/ Docker + gVisor/Kata, autoscaled]
                          │
   Postgres (managed)  +  object storage (S3/GCS) for archives & logs
```
- API tier becomes truly stateless (reads/writes DB + object storage only).
- Judge workers own the Docker/sandbox concern and scale independently with queue depth.
- Artifacts and logs move to object storage so any API replica can serve `/source` and
  `/logs/{stage}`.

---

## Prioritized checklist

- [ ] **P0** Override default admin creds from secret; `GIN_MODE=release`
- [ ] **P0** TLS at ingress; never expose `:8080` directly
- [ ] **P0** Secrets in a manager; DB `sslmode=require`
- [ ] **P0** Stronger sandbox (gVisor/Kata/Firecracker) on isolated judge nodes
- [ ] **P1** Decide storage model: single node **or** object storage / shared RWX volume
- [ ] **P1** Externalize the queue + split API / judge worker if scaling > 1 node
- [ ] **P1** Versioned DB migrations instead of boot-time `AutoMigrate`
- [ ] **P2** Structured logs, metrics, `/readyz`, rate limiting, CORS, CI/CD

---

## Bottom line

The codebase is well-structured and the *application* is cloud-ready. What is **not** ready is
the **deployment topology**: the judge engine assumes a Docker host and a shared local
filesystem. Pick the model deliberately — **Option A** to ship quickly on one VM, **Option B**
when you need to scale — and clear the P0 list before exposing it to the internet.
