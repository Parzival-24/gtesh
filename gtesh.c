#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define MAX_PATHS 100  
#define MAX_COMMANDS 20  // Máximo de comandos paralelos

char *paths[MAX_PATHS];
int path_count = 0;

typedef enum { MODE_INTERACTIVE, MODE_BATCH } ShellMode;
ShellMode mode;

// Prototipos
void run_interactive(FILE *input);
void run_batch(FILE *input);
void execute_command(char **args);
void execute_parallel_commands(char *input);
void print_error(void);
char **parse_command(char *line);
void builtin_path(char **args);
char *resolve_command(char *cmd);
static void clear_path(void);
void trim_whitespace(char *str);

//______________MAIN_______________//
int main(int argc, char *argv[]) {
    FILE *input = NULL;
    paths[0] = strdup("/bin");
    path_count = 1;

    // VALIDACION DE ARGUMENTOS INGRESADOS
    if (argc == 1) {
        mode = MODE_INTERACTIVE;
        input = stdin;
    } else if (argc == 2) {
        mode = MODE_BATCH;
        input = fopen(argv[1], "r");
        if (input == NULL) {
            perror("Error al abrir el archivo de batch");
            exit(1);
        }
    } else {
        fprintf(stderr, "Uso: %s [batch_file]\n", argv[0]);
        exit(1);
    }

    // LOGICA PRINCIPAL DE MAIN
    if (mode == MODE_INTERACTIVE) {
        run_interactive(input);
    } else {
        run_batch(input);
    }

    if (mode == MODE_BATCH) {
        fclose(input);
    }
    return 0;
}

// METODO PARA EJECUTAR COMANDOS PARALELOS
void execute_parallel_commands(char *input) {
    pid_t pids[MAX_COMMANDS];
    char *commands[MAX_COMMANDS];
    int command_count = 0;
    
    // Dividir por & pero preservar comandos
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, "&");
    
    while (token != NULL && command_count < MAX_COMMANDS) {
        trim_whitespace(token);
        if (strlen(token) > 0) {
            commands[command_count] = strdup(token);
            command_count++;
        }
        token = strtok(NULL, "&");
    }
    
    // Ejecutar todos los comandos en paralelo
    for (int i = 0; i < command_count; i++) {
        // Parsear argumentos para cada comando
        char **args = parse_command(commands[i]);
        
        if (args[0] != NULL) {
            // Built-in commands se ejecutan en el proceso padre
            if (strcmp(args[0], "exit") == 0) {
                free(args);
                // Liberar memoria antes de salir
                for (int j = 0; j < command_count; j++) {
                    free(commands[j]);
                }
                free(input_copy);
                exit(0);
            } else if (strcmp(args[0], "path") == 0) {
                builtin_path(args);
                free(args);
                continue;  // No crear proceso para built-in
            }
            
            // Para comandos externos, crear proceso hijo
            pids[i] = fork();
            
            if (pids[i] == 0) {
                // Proceso hijo
                if (path_count == 0) {
                    print_error();
                    _exit(1);
                }
                
                char *prog = resolve_command(args[0]);
                if (prog != NULL) {
                    execv(prog, args);
                    free(prog);
                }
                print_error();
                _exit(1);
            } else if (pids[i] < 0) {
                print_error();
            }
        }
        free(args);
    }
    
    // ESPERAR a que TODOS los procesos terminen
    for (int i = 0; i < command_count; i++) {
        if (pids[i] > 0) {
            int status;
            waitpid(pids[i], &status, 0);
        }
        free(commands[i]);
    }
    
    free(input_copy);
}

// METODO PARA LIMPIAR WHITESPACE
void trim_whitespace(char *str) {
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
}

// LOS DEMÁS MÉTODOS PERMANECEN IGUALES
static void clear_path(void) {
    for (int i = 0; i < path_count; i++) {
        free(paths[i]);
        paths[i] = NULL;
    }
    path_count = 0;
}

void execute_command(char **args) {
    if (args == NULL || args[0] == NULL) return;

    // Built-in: path
    if (strcmp(args[0], "path") == 0) {
        builtin_path(args);
        return;
    }

    // Si PATH vacío, no ejecutar externos
    if (path_count == 0) {
        print_error();
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        print_error();
        return;
    }
    if (pid == 0) {
        char *prog = resolve_command(args[0]);
        if (prog != NULL) {
            execv(prog, args);
            free(prog);
        }
        print_error();
        _exit(1);
    } else { 
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            print_error();
        }
    }
}

void print_error(void) {
    const char msg[] = "An error has occurred\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}

char **parse_command(char *line) {
    char **args = malloc(MAX_ARGS * sizeof(char *));
    int i = 0;
    char *token;

    while ((token = strsep(&line, " \t")) != NULL) {
        if (strlen(token) == 0) continue; 
        args[i++] = token;
    }
    args[i] = NULL; 
    return args;
}

// MODIFICAR run_interactive Y run_batch PARA DETECTAR COMANDOS PARALELOS
void run_interactive(FILE *input) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while (1) {
        printf("gtesh> ");
        fflush(stdout);

        read = getline(&line, &len, input);
        if (read == -1) break;

        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        // Verificar si hay operador & para comandos paralelos
        if (strchr(line, '&') != NULL) {
            execute_parallel_commands(line);
        } else {
            char **args = parse_command(line);
            if (args[0] != NULL) {
                if (strcmp(args[0], "exit") == 0) {
                    free(args);
                    break;
                }
                execute_command(args); 
            }
            free(args);
        }
    }
    free(line);
}

void run_batch(FILE *input) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while (1) {
        read = getline(&line, &len, input);
        if (read == -1) break;

        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }

        // Verificar si hay operador & para comandos paralelos
        if (strchr(line, '&') != NULL) {
            execute_parallel_commands(line);
        } else {
            char **args = parse_command(line);
            if (args[0] != NULL) {
                if (strcmp(args[0], "exit") == 0) {
                    free(args);
                    break;
                }
                execute_command(args); 
            }
            free(args);
        }
    }
    free(line);
}

void builtin_path(char **args) {
    clear_path();
    for (int i = 1; args[i] != NULL && path_count < MAX_PATHS; i++) {
        paths[path_count++] = strdup(args[i]);
    }
}

char *resolve_command(char *cmd) {
    if (cmd == NULL) return NULL;

    // Caso 1: ruta explícita (contiene '/')
    if (strchr(cmd, '/')) {
        if (access(cmd, X_OK) == 0) {
            return strdup(cmd);
        } else {
            return NULL;
        }
    }

    // Caso 2: buscar en cada directorio de PATH
    for (int i = 0; i < path_count; i++) {
        char candidate[512];
        snprintf(candidate, sizeof(candidate), "%s/%s", paths[i], cmd);

        if (access(candidate, X_OK) == 0) {
            return strdup(candidate); 
        }
    }
    return NULL;
}