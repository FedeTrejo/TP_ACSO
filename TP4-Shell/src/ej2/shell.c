#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <termios.h>

#define MAX_COMMANDS 200    
#define MAX_ARGS 65       

/* ------------------------------------------------------------
 * Manejador de SIGCHLD
 * Recolecta a los hijos finalizados para evitar zombies
 * ----------------------------------------------------------*/
static void sigchld_handler(int signo) {
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

/* ------------------------------------------------------------
 * Detecta si la línea quedó con comillas sin cerrar
 * Devuelve true si falta la comilla de cierre
 * ----------------------------------------------------------*/
static bool tiene_comilla_abierta(const char *s){
    char q = 0;
    for(const char *p = s; *p; ++p){
        if(q){
            if(*p == '\\' && *(p+1) == q){ ++p; continue; }
            if(*p == q) q = 0;
        }else if(*p == '\'' || *p == '"') q = *p;
    }
    return q != 0;
}

/* ------------------------------------------------------------
 * Verifica sintaxis de tuberías (ignora | dentro de comillas)
 * true = error de sintaxis
 * ----------------------------------------------------------*/
static bool sintaxis_pipe_invalida(const char *s){
    char q = 0;
    int ultimo = -1; /* 0=espacio, 1=palabra, 2=pipe */

    for(const char *p = s; *p; ++p){
        if(q){
            if(*p == '\\' && *(p+1) == q){ ++p; continue; }
            if(*p == q) q = 0;
            continue;
        }
        if(*p == '\'' || *p == '"'){ q = *p; continue; }
        if(isspace((unsigned char)*p)) continue;
        if(*p == '|'){
            if(ultimo != 1) return true; 
            ultimo = 2;
        }else{
            ultimo = 1;
        }
    }
    return ultimo == 2; 
}

/* ------------------------------------------------------------
 * Divide la línea en etapas separadas por |
 * commands apunta a cada segmento
 * ----------------------------------------------------------*/
static void dividir_por_pipes(char *line, char *commands[], int *n_commands) {
    int count = 0;
    char *start = line;
    char *p = line;
    char quote = 0;

    while (*p) {
        if (quote) {
            if (*p == '\\' && *(p+1) == quote) {
                p += 2;
                continue;
            }
            if (*p == quote) {
                quote = 0;
            }
            p++;
        } else {
            if (*p == '\'' || *p == '"') {
                quote = *p++;
            }
            else if (*p == '|') {
                char *seg_start = start;
                char *seg_end   = p - 1;
                while (seg_start <= seg_end && isspace((unsigned char)*seg_start)) {
                    seg_start++;
                }
                while (seg_end >= seg_start && isspace((unsigned char)*seg_end)) {
                    *seg_end-- = '\0';
                }
                if (*seg_start) {
                    commands[count++] = seg_start;
                }
                *p = '\0';  
                p++;
                start = p;
            } else {
                p++;
            }
        }
    }
    // El último segmento desde start hasta \0
    char *seg_start = start;
    while (*seg_start && isspace((unsigned char)*seg_start)) {
        seg_start++;
    }
    char *seg_end = seg_start + strlen(seg_start) - 1;
    while (seg_end >= seg_start && isspace((unsigned char)*seg_end)) {
        *seg_end-- = '\0';
    }
    if (*seg_start) {
        commands[count++] = seg_start;
    }
    *n_commands = count;
}

/* ------------------------------------------------------------
 * Tokeniza una etapa en argv (maneja comillas y escapes)
 * ----------------------------------------------------------*/
static int parsear_args(char *cmd, char **argv, bool *overflow) {
    int argc = 0;
    char *p = cmd;
    *overflow = false;

    while (*p) {
        // Saltar espacios en blanco 
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        // Si ya se llegó al límite de tokens permitidos, overflow 
        if (argc >= MAX_ARGS - 1) {
            *overflow = true;
            return argc;
        }

        char quote = 0;
        if (*p == '\'' || *p == '"') {
            quote = *p++;
        }

        char *start = p;
        int token_len = 0;

        if (quote) {
            // Contamos hasta la comilla de cierre, respetando escapes 
            while (*p) {
                if (*p == '\\' && *(p+1) == quote) {
                    p += 2;
                    token_len++;
                    continue;
                }
                if (*p == quote) {
                    break;
                }
                p++;
                token_len++;
            }
        } else {
            // Sin comillas, contamos hasta el siguiente whitespace 
            while (*p && !isspace((unsigned char)*p)) {
                p++;
                token_len++;
            }
        }

        char *token = malloc(token_len + 1);
        if (!token) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        // Copiamos carácter a carácter, procesando escapes si hay comillas 
        char *q = start;
        int i = 0;
        if (quote) {
            while (i < token_len && *q) {
                if (*q == '\\' && *(q+1) == quote) {
                    token[i++] = quote;
                    q += 2;
                }
                else if (*q == quote) {
                    break;
                }
                else {
                    token[i++] = *q++;
                }
            }
            if (*q == quote) {
                q++;
            }
        } else {
            while (i < token_len && *q && !isspace((unsigned char)*q)) {
                token[i++] = *q++;
            }
        }
        token[i] = '\0';

        argv[argc++] = token;

        // Si había comilla, avanzamos p hasta después del cierre 
        if (quote) {
            p = q;
        }
    }

    argv[argc] = NULL;
    return argc;
}

static void liberar_argv(char **argv){ for(int i = 0; argv[i]; ++i) free(argv[i]); }

int main(void){

    char command[1024];
    char *commands[MAX_COMMANDS];
    int command_count;

    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) < 0){ perror("sigaction"); return 1; }

    while(1){
        if(isatty(STDIN_FILENO)){ printf("Shell> "); fflush(stdout); } //para tests

        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        if(!fgets(command, sizeof command, stdin)){
            if(isatty(STDIN_FILENO)) putchar('\n');
            break;
        }

        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
        This is done by replacing the newline character with the null character ('\0').
        The strcspn() function returns the length of the initial segment of command that consists of 
        characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';
        
        if(!*command) continue;

        if(tiene_comilla_abierta(command)){
            fprintf(stderr, "shell: comilla sin cerrar\n");
            continue;
        }
        if(sintaxis_pipe_invalida(command)){
            fprintf(stderr, "shell: error de sintaxis cerca de '|'\n");
            continue;
        }

        // Built‑ins: exit / cd (solo si no hay pipes) 
        {
            bool of = false; char *tmp = strdup(command);
            char *av[MAX_ARGS]; int ac = parsear_args(tmp, av, &of);
            if(ac > 0 && strcmp(av[0], "exit") == 0 && !strchr(command, '|')){
                liberar_argv(av); free(tmp); break; }
            if(ac > 0 && strcmp(av[0], "cd") == 0 && !strchr(command, '|')){
                const char *dest = (ac == 1) ? getenv("HOME") : av[1];
                if(!dest) dest = "/";
                if(chdir(dest) < 0) perror("cd");
                liberar_argv(av); free(tmp); continue; }
            liberar_argv(av); free(tmp);
        }

        // Se ejecuta en background? 
        bool bg = false; size_t len = strlen(command);
        if(len && command[len-1] == '&'){
            bg = true; command[--len] = '\0';
            while(len && isspace((unsigned char)command[len-1])) command[--len] = '\0';
            if(!len) continue;
        }

        dividir_por_pipes(command, commands, &command_count);
        if(command_count == 0) continue;

        int pipes[MAX_COMMANDS][2];
        for(int i = 0; i < command_count - 1; i++)
            if(pipe(pipes[i]) < 0){ perror("pipe"); exit(1); }

        pid_t pgid = 0, pids[MAX_COMMANDS];

        for(int i = 0; i < command_count; i++){
            pid_t pid = fork();
            if(pid < 0){ perror("fork"); exit(1); }
            if(pid == 0){ /* ------------ HIJO ------------ */
                if(i == 0){ pgid = getpid(); setpgid(0, pgid); if(!bg) tcsetpgrp(STDIN_FILENO, pgid); }
                else setpgid(0, pgid);

                if(i > 0)           dup2(pipes[i-1][0], STDIN_FILENO);
                if(i < command_count-1) dup2(pipes[i][1], STDOUT_FILENO);
                for(int k = 0; k < command_count - 1; k++){ close(pipes[k][0]); close(pipes[k][1]); }

                char *argv[MAX_ARGS]; bool of = false; int argc = parsear_args(commands[i], argv, &of);
                if(argc == 0){ fprintf(stderr, "shell: comando vacío\n"); _exit(1); }
                if(of){ fprintf(stderr, "shell: demasiados argumentos\n"); _exit(1); }

                int in_fd = -1, out_fd = -1;
                for(int j = 0; j < argc; j++){
                    if(strcmp(argv[j], "<") == 0 && j+1 < argc){ in_fd = open(argv[j+1], O_RDONLY); goto redir; }
                    if(strcmp(argv[j], ">>") == 0 && j+1 < argc){ out_fd = open(argv[j+1], O_WRONLY|O_CREAT|O_APPEND, 0666); goto redir; }
                    if(strcmp(argv[j], ">") == 0 && j+1 < argc){ out_fd = open(argv[j+1], O_WRONLY|O_CREAT|O_TRUNC, 0666); goto redir; }
                    continue;
redir:
                    if(in_fd < 0 && out_fd < 0){ fprintf(stderr, "shell: error de redirección\n"); _exit(1); }
                    free(argv[j]); free(argv[j+1]);
                    for(int k = j; k + 2 <= argc; k++) argv[k] = argv[k+2];
                    argc -= 2; j--; }
                argv[argc] = NULL;

                if(in_fd >= 0){ dup2(in_fd, STDIN_FILENO); close(in_fd); }
                if(out_fd >= 0){ dup2(out_fd, STDOUT_FILENO); close(out_fd); }

                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);

                execvp(argv[0], argv);
                fprintf(stderr, "shell: %s: comando no encontrado\n", argv[0]); _exit(1);
            }
            /* ------------ PADRE ------------ */
            if(i == 0){ pgid = pid; setpgid(pid, pgid); if(!bg) tcsetpgrp(STDIN_FILENO, pgid); }
            else setpgid(pid, pgid);
            pids[i] = pid;
        }

        for(int i = 0; i < command_count - 1; i++){ close(pipes[i][0]); close(pipes[i][1]); }

        if(!bg){
            for(int i = 0; i < command_count; i++) waitpid(pids[i], NULL, 0);
            tcsetpgrp(STDIN_FILENO, getpid());
        }else{
            printf("[Proceso en background PID %d]\n", pids[command_count - 1]);
        }
    }
    return 0;
}
