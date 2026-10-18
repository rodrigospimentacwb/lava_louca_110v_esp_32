#!/usr/bin/env bash
# Gera build/compile_commands.json para o autocomplete do VS Code (extensão C/C++).
#
# O arduino-cli compila CÓPIAS dos arquivos em build/sketch/. Este script troca
# esses caminhos pelos arquivos reais em LavaLoucas/, que são os que você edita.
# Rode de novo quando criar ou renomear arquivos.
set -euo pipefail
cd "$(dirname "$0")/.."

arduino-cli compile --fqbn esp32:esp32:esp32 --build-path build --only-compilation-database LavaLoucas >/dev/null

python3 -I - "$PWD" <<'PY'
import json, sys
root = sys.argv[1]
path = f"{root}/build/compile_commands.json"
sketch = f"{root}/build/sketch/"
entries = json.load(open(path))
for e in entries:
    if e["file"].startswith(sketch):
        rel = e["file"][len(sketch):]
        if rel == "LavaLoucas.ino.cpp":
            rel = "LavaLoucas.ino"
        e["file"] = f"{root}/LavaLoucas/{rel}"
json.dump(entries, open(path, "w"), indent=1)
print("ok:", path)
PY
