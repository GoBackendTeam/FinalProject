.PHONY: keys db up down run build tidy test

keys:
	bash scripts/gen_keys.sh ./keys

db:
	docker compose up -d db

down:
	docker compose down

tidy:
	go mod tidy

build:
	go build -o bin/server ./cmd/server

run: keys
	go run ./cmd/server

test:
	go test ./...
