.PHONY: keys db up down run build tidy test

keys:
	bash scripts/gen_keys.sh ./keys

db:
	docker compose up -d db

up: keys
	docker compose up -d --build

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
