#!/bin/bash
# test_error_messages.sh — Verifica que el mensaje de error sea exactamente igual en todos los casos

ROOT_DIR=$(pwd)
TEST_DIR="$ROOT_DIR/test_files"
RESULT_DIR="$ROOT_DIR/test_errors"

mkdir -p "$RESULT_DIR"

# Mensaje esperado (exacto, con newline)
EXPECTED_MSG="An error has occurred"

verify_error() {
  local name="$1"
  local cmd="$2"
  local description="$3"
  
  printf "%-30s" "[$name]"
  
  # Ejecutar comando y capturar stderr
  eval "$cmd" 2> "$RESULT_DIR/${name}.err" > "$RESULT_DIR/${name}.out" || true
  
  # Leer stderr
  actual=$(cat "$RESULT_DIR/${name}.err")
  
  # Verificar si contiene exactamente el mensaje
  if [ "$actual" = "$EXPECTED_MSG" ]; then
    echo "✓ PASS"
    return 0
  else
    echo "✗ FAIL"
    echo "  Descripción: $description"
    echo "  Esperado: [$EXPECTED_MSG]"
    echo "  Actual:   [$actual]"
    return 1
  fi
}

echo "=========================================="
echo "PRUEBAS DE MENSAJE DE ERROR"
echo "=========================================="
echo ""

passed=0
failed=0

# CASO 1: Redirección múltiple (múltiples archivos después de >)
if verify_error "redir_multi_files" \
  "$ROOT_DIR/gtesh $TEST_DIR/batch_redir_error.txt" \
  "Comando: /bin/ls > out2 out3"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 2: PATH vacío (intenta ejecutar binario sin PATH)
if verify_error "path_empty" \
  "$ROOT_DIR/gtesh $TEST_DIR/batch_path_empty.txt" \
  "PATH vacío; intento de ejecutar /bin/echo"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 3: cd con múltiples argumentos
if verify_error "cd_multiple_args" \
  "$ROOT_DIR/gtesh $TEST_DIR/batch_cd_error.txt" \
  "Comando: cd /tmp /usr"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 4: cd a directorio inexistente
if verify_error "cd_nonexistent" \
  "printf 'cd /nonexistent_dir_xyz_12345\nexit\n' | $ROOT_DIR/gtesh" \
  "Intento cd a ruta que no existe"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 5: Comando no encontrado (no en PATH)
if verify_error "cmd_not_found" \
  "printf 'path /bin\n/nonexistent_command_xyz_12345\nexit\n' | $ROOT_DIR/gtesh" \
  "Comando inexistente no está en PATH"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 6: Redirección sin nombre de archivo (> sin argumento)
if verify_error "redir_no_filename" \
  "printf 'echo hello >\nexit\n' | $ROOT_DIR/gtesh" \
  "Redirección sin archivo: echo hello >"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 7: Invocación con más de un argumento
if verify_error "too_many_args" \
  "$ROOT_DIR/gtesh file1.txt file2.txt" \
  "Invocación con 2+ argumentos (error al iniciar)"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

# CASO 8: Archivo batch inexistente
if verify_error "batch_not_found" \
  "$ROOT_DIR/gtesh /nonexistent_batch.txt" \
  "Archivo batch no existe"; then
  ((passed++))
else
  ((failed++))
fi
echo ""

echo "=========================================="
echo "RESUMEN"
echo "=========================================="
echo "✓ Pasadas: $passed"
echo "✗ Fallos:  $failed"
echo "Total:    $((passed + failed))"
echo ""
echo "Archivos de resultado guardados en: $RESULT_DIR/"
echo ""

# Mostrar contenido exacto (en hex) de un caso para inspección
echo "Inspección de bytes (stderr - caso redir_multi_files):"
od -c "$RESULT_DIR/redir_multi_files.err"
