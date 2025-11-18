#!/bin/bash
# Script para ejecutar todas las pruebas de gtesh y guardar resultados en test_results/
# Ejecútalo dentro del directorio del proyecto (donde está el ejecutable `gtesh`).

set -o errexit
set -o nounset

ROOT_DIR=$(pwd)
TEST_DIR="$ROOT_DIR/test_files"
RESULT_DIR="$ROOT_DIR/test_results"

mkdir -p "$RESULT_DIR"

run() {
  local name="$1"
  local file="$2"
  echo "--- Ejecutando: $name"
  # Guardar stdout y stderr por separado
  "$ROOT_DIR/gtesh" "$file" > "$RESULT_DIR/${name}.stdout" 2> "$RESULT_DIR/${name}.stderr" || true
  echo "$?" > "$RESULT_DIR/${name}.exit"
}

# Lista de pruebas
run basic "$TEST_DIR/batch_basic.txt"
run parallel "$TEST_DIR/batch_parallel.txt"
run redir_error "$TEST_DIR/batch_redir_error.txt"
run cd "$TEST_DIR/batch_cd.txt"
run path_empty "$TEST_DIR/batch_path_empty.txt"
run cd_multi_arg "$TEST_DIR/batch_cd_error.txt"
run path_test "$TEST_DIR/batch_path_test.txt"

# Resumen breve
echo "\nResumen de resultados (salidas en $RESULT_DIR):"
for f in "$RESULT_DIR"/*.exit; do
  base=$(basename "$f" .exit)
  code=$(cat "$f")
  echo "- $base : exit=$code"
done

echo "Hecho. Revisa los archivos en $RESULT_DIR"
