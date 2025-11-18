# gtesh — Shell Básico (IF-4001 Sistemas Operativos)

Intérprete de línea de comandos (shell) implementado en C que soporta modo interactivo, modo batch, comandos integrados, redirección de salida y ejecución de comandos en paralelo.

## Compilación

```bash
gcc -std=gnu11 -Wall -Wextra -o gtesh gtesh.c
```

## Uso

### Modo interactivo
```bash
./gtesh
```
Muestra el prompt `gtesh>` y espera comandos del usuario hasta que escriba `exit`.

### Modo batch
```bash
./gtesh batch_file.txt
```
Lee comandos de `batch_file.txt` y los ejecuta sin mostrar prompt.

## Características implementadas

### Comandos integrados
- **exit**: Termina el shell con código 0
- **cd**: Cambia de directorio (`cd [ruta]`)
- **path**: Define la ruta de búsqueda de comandos (`path /bin /usr/bin ...`)

### Redirección de salida
```bash
comando > archivo
```
Redirige stdout y stderr del comando al archivo especificado. Si el archivo existe, se trunca.

### Comandos paralelos
```bash
cmd1 & cmd2 & cmd3
```
Ejecuta múltiples comandos en paralelo. El shell espera a que todos terminen antes de continuar.

### Búsqueda en PATH
- Ruta inicial: `/bin`
- Busca comandos en los directorios especificados por `path`
- Usa `access(cmd, X_OK)` para verificar ejecutabilidad

### Manejo de errores
Imprime exactamente `An error has occurred\n` en stderr para cualquier error de sintaxis o ejecución.

## Pruebas

### Compilación previa
```bash
gcc -std=gnu11 -Wall -Wextra -o gtesh gtesh.c
```

### Ejecutar todas las pruebas automaticamente
```bash
chmod +x test_files/test_error_messages.sh
./test_files/test_error_messages.sh
```

### Ejecutar pruebas de funcionalidad una a una

#### 1. Prueba básica (echo, redirección, pwd)
```bash
./gtesh test_files/batch_basic.txt
cat out1.txt  # Verificar que contiene "world"
```
**Valida:** Funcionalidad básica, Redirección de Salida, Búsqueda y Ejecución

#### 2. Prueba de paralelismo
```bash
./gtesh test_files/batch_parallel.txt
```
**Valida:** Comandos Paralelos, Manejo de procesos

#### 3. Prueba de redirección con error (múltiples archivos)
```bash
./gtesh test_files/batch_redir_error.txt 2>&1 | grep "An error has occurred"
```
**Valida:** Robustez y manejo de errores, Redirección de Salida

#### 4. Prueba de cd y path
```bash
./gtesh test_files/batch_cd.txt 2>&1
```
**Valida:** Comandos incorporados (cd, path), Robustez

#### 5. Prueba de PATH vacío
```bash
./gtesh test_files/batch_path_empty.txt 2>&1 | grep "An error has occurred"
```
**Valida:** Búsqueda y Ejecución, Robustez

#### 6. Prueba de cd con múltiples argumentos (error)
```bash
./gtesh test_files/batch_cd_error.txt 2>&1 | grep "An error has occurred"
```
**Valida:** Comandos incorporados, Robustez

#### 7. Prueba de PATH con múltiples directorios
```bash
./gtesh test_files/batch_path_test.txt
head -n 5 out_ls.txt  # Verificar contenido del archivo redirigido
```
**Valida:** Búsqueda y Ejecución, Redirección de Salida

#### 8. Prueba de modo interactivo
```bash
printf "path /bin\n/bin/echo Hola\nexit\n" | ./gtesh
```
**Valida:** Funcionalidad básica (modo interactivo), Comandos incorporados

#### 9. Validación del mensaje de error exacto
```bash
./gtesh test_files/batch_redir_error.txt 2> tmp_err.txt 1>/dev/null
cat tmp_err.txt
# Debe mostrar exactamente: An error has occurred
```
**Valida:** Robustez y manejo de errores

### Limpiar archivos generados por las pruebas
```bash
rm -f out1.txt out_ls.txt tmp_err.txt
rm -rf test_results
```

### Casos de prueba incluidos

**test_files/batch_basic.txt**
- Prueba: echo, redirección de salida, pwd
- Rúbrica: Funcionalidad básica, Redirección

**test_files/batch_parallel.txt**
- Prueba: ejecución de comandos con `&`
- Rúbrica: Comandos paralelos, Manejo de procesos

**test_files/batch_redir_error.txt**
- Prueba: redirección múltiple (error)
- Rúbrica: Robustez y manejo de errores

**test_files/batch_cd.txt**
- Prueba: path, cd correcto, cd fallido
- Rúbrica: Comandos incorporados, Robustez

**test_files/batch_path_empty.txt**
- Prueba: PATH vacío (no se puede ejecutar externos)
- Rúbrica: Búsqueda y Ejecución

**test_files/batch_cd_error.txt**
- Prueba: cd con múltiples argumentos
- Rúbrica: Comandos incorporados, Robustez

**test_files/batch_path_test.txt**
- Prueba: PATH con múltiples directorios, búsqueda relativa
- Rúbrica: Búsqueda y Ejecución, Redirección

### Script de pruebas de error
**test_files/test_error_messages.sh**

Valida que el mensaje de error sea exactamente `An error has occurred\n` en 8 casos diferentes:
1. Redirección múltiple
2. PATH vacío
3. cd con múltiples argumentos
4. cd a directorio inexistente
5. Comando no encontrado
6. Redirección sin archivo
7. Invocación con 2+ argumentos
8. Archivo batch no existe

## Criterio de evaluación (rúbrica del proyecto)

| Criterio | Ponderación |
|----------|-------------|
| Funcionalidad básica | 25% |
| Manejo de procesos | 20% |
| Búsqueda y Ejecución | 15% |
| Comandos incorporados | 10% |
| Comandos Paralelos | 10% |
| Redirección de Salida | 10% |
| Robustez y manejo de errores | 10% |

## Cambios realizados

### Correcciones de errores
1. Cambiar mensajes de error personalizados a `print_error()` en main()
2. Cambiar `return` a `_exit(1)` en proceso hijo cuando falla apertura de archivo
3. Convertir line endings de CRLF (Windows) a LF (Unix) en archivos batch

### Validaciones completadas
✓ Todos los 8 casos de error imprimen exactamente `An error has occurred\n`  
✓ Compilación sin advertencias en modo -Wall -Wextra  
✓ Funcionalidad de builtins (exit, cd, path) validada  
✓ Redirección de salida funcional  
✓ Ejecución de comandos paralelos correcta  
✓ Búsqueda en PATH implementada correctamente  

## Autores
- David Avila Torres C10789
- Kristel Samanta De Franco Navarro C4E644
- Krystell Membreño Marchena C4H089
- Gerald Salazar Hernandez C07126
