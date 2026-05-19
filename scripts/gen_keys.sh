#!/usr/bin/env bash
# 產生 JWT 簽章用的 EC P-256 金鑰對(對應 PRD 指定指令)。
set -euo pipefail

KEYS_DIR="${1:-./keys}"
mkdir -p "$KEYS_DIR"

if [[ -f "$KEYS_DIR/private.pem" ]]; then
  echo "keys already exist at $KEYS_DIR, skip (delete to regenerate)" >&2
  exit 0
fi

openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:P-256 -out "$KEYS_DIR/private.pem"
openssl pkey -in "$KEYS_DIR/private.pem" -pubout -out "$KEYS_DIR/public.pem"
chmod 600 "$KEYS_DIR/private.pem"

echo "generated:"
echo "  $KEYS_DIR/private.pem"
echo "  $KEYS_DIR/public.pem"
