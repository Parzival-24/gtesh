#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAX_INPUT 1024
#define MAX_ARGS 100
#define MAX_PATHS 100  
char *paths[MAX_PATHS];
int path_count = 0;
typedef enum { MODE_INTERACTIVE, MODE_BATCH } ShellMode;
ShellMode mode; //ESTA VARIABLE ES PARA ELEGIR EL MODO (INTERACTIVO O LOTES)


// Prototipos(Define los metodos globales)
void run_interactive(FILE *input);
void run_batch(FILE *input);
void execute_command(char **args);
void print_error(void);
char **parse_command(char *line);
void builtin_path(char **args);
char *resolve_command(char *cmd);






//______________MAIN_______________//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//

int main(int argc, char *argv[]) {
    FILE *input = NULL; //Esta variable representa la FUENTE de entrada (Interactivo o lotes)
    char *line = NULL;//Expande memoria si hace falta
    size_t len = 0;//Guarda el tamaño actual asignado de buffer
    ssize_t read;//Guarda el numero de caracteres Leidos por getline
    paths[0] = strdup("/bin");
    path_count = 1;  // Requisito: ruta inicial debe contener /bin

    // VALIDACION DE ARGUMENTOS INGRESADOS
    //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//

    //MODE INTERACTIVO//
    //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
    if (argc == 1) {
        mode = MODE_INTERACTIVE;
        input = stdin;
    } else if (argc == 2) {
    //MODE LOTES//
    //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
        mode = MODE_BATCH;
        input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error al abrir el archivo de batch");
        exit(1);
        }
    //MODE INCORRECTO (DOS ARGUMENTOS)//
    //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
    } else {
        fprintf(stderr, "Uso: %s [batch_file]\n", argv[0]);
        exit    (1);
    }

    //LOGICA PRINCIPAL DE MAIN//
    //↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
    if (mode == MODE_INTERACTIVE) {
        run_interactive(input);
    } else {
        run_batch(input);
    }

    if (mode == MODE_BATCH) {
        fclose(input);
    }
    return 0;
}//FIN DEL MAIN





//METODO ESTATICO PARA LIMPIAR LA RUTA(PATH)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
static void clear_path(void) {
    for (int i = 0; i < path_count; i++) {
        free(paths[i]);
        paths[i] = NULL;
    }
    path_count = 0;
}
 
//METODO PARA IMPLEMENTAR FORK Y EXECV//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
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
        if (prog != NULL)
        {
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

//METODO PARA MENSAJE DE ERROR//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
void print_error(void) {
    const char msg[] = "An error has occurred\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}

//METODO PARA RECIBIR LINEA DE ARGUMENTOS//
//PARA LUEGO GENERAR ARREGLO DE PUNTERO(TOKENS)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
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

//METODO PARA CUANDO EL MODO ES INTERACTIVO//
//(EL USUARIO SOLO INGRESO UN ARGUMENTO)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
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

    free(line);
}

//METODO PARA CUANDO EL MODO ES LOTES/BATCH//
//(EL USUARIO INGRESO DOS ARGUMENTO)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
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

    free(line);
}

//METODO PARA sobrescribe la lista (PATH)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
void builtin_path(char **args) {
    clear_path();  // “path” SIEMPRE sobrescribe
    for (int i = 1; args[i] != NULL && path_count < MAX_PATHS; i++) {
        paths[path_count++] = strdup(args[i]);
    }
    // Si el usuario dejó 'path' sin args, path_count=0 → PATH vacío (solo built-ins)
}

//METODO PARA Resolver el ejecutable 
//(buscar en PATH o usar ruta explícita)//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//
char *resolve_command(char *cmd) {
    if (cmd == NULL) return NULL;

    // Caso 1: ruta explícita (contiene '/')
    if (strchr(cmd, '/')) {
        if (access(cmd, X_OK) == 0) {
            return strdup(cmd);  // es ejecutable
        } else {
            return NULL;         // ruta inválida o sin permisos
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