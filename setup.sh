#!/usr/bin/env sh
# Prepare your own Naruto: Rise of a Ninja disc for the PS5 app (Linux or WSL2).
# Usage: sh setup.sh --iso "/path/to/Naruto - Rise of a Ninja.iso" [--output DIR/PPSA99680]
set -eu
exec python3 "$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)/ps5/tools/setup.py" "$@"
