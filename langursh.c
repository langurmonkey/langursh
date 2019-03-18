
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#include "langursh.h"
#include "parser.h"


/* home directory */
char *homedir;

/* built-in commands */
const char *builtin_cmd[] = { 
    "cd", 
    "history",
    "!",
    "help", 
    "exit",
    "quit"
};
const char *builtin_cmd_desc[] = { 
    "change directory", 
    "print history",
    "run history command (!2, !-1)",
    "print this help", 
    "exit the shell",
    "exit the shell"
};

/* built-in functions */
const int (*builtin_fnc[]) (char **) = {
    &run_cd,
    &run_print_history,
    &run_history,
    &run_help,
    &run_exit,
    &run_exit
};

/* number of built-in commands */
const int n_builtin = sizeof(builtin_cmd) / sizeof(builtin_cmd[0]);

/* command history */
char *history[HISTORY_SIZE];
/* current history size */
int hist_size = 0;
/* current start index */
int hist_i = 0;

int main(int argc, char **argv){
    int r_state = 1;
    int *running = &r_state;

    char *line;
    char **tokens;

   
    printf("Langur Shell (langursh), version %s\n", VERSION);
    printf("Copyright © 2019 Toni Sagristà Sellés\n\n");
   
    struct passwd *pw = getpwuid(getuid());
    homedir = pw->pw_dir;


    while(*running == 1){
        /* print the prompt */
        print_prompt();

        /* read user line */
        line = read_line();
        char *linecpy = str_copy(line);

        /* tokenize */
        tokens = tokenize_line(line);

        /* act */
        int ret;
        if(*tokens && (ret = act(tokens)) == 0) {
            // exit
            *running = 0;
        }

        /* add to history */
        if(*tokens && ret == 1) {
            free(history[hist_i]);
            history[hist_i] = linecpy;
            hist_i = (hist_i + 1) % HISTORY_SIZE;
            hist_size = min(HISTORY_SIZE, hist_size + 1);
        }
    }

    dispose();
    return EXIT_SUCCESS;
}

void dispose() {
    /* free history */
    for (int i = 0; i < hist_size; i++){
        free(history[i]);
    }
}

char *str_copy(char *str) {
    const size_t len_str = strlen(str) + 1;
    char *my_copy = malloc(len_str);
    strncpy(my_copy, str, len_str);
    return my_copy;
}

int act(char **tokens) {
    for(int i = 0; i < n_builtin; i++){
        if(strcmp(tokens[0], builtin_cmd[i]) == 0) {  
            // we have a match
            return (*builtin_fnc[i])(tokens);
        } else if (starts_with(tokens[0], builtin_cmd[i])) {
            // starts with, only support first character
            return (*builtin_fnc[i])(tokens);
        }
    }
    // no built-in command, run process
    return run_process(tokens);
}

int run_line(char *line) { 
    char *linecpy = str_copy(line);
    /* tokenize */
    char **tokens = tokenize_line(linecpy);

    /* act */
    if(*tokens)
        act(tokens);

    /* free */
    free(linecpy);

    return 1;
}

int run_exit(char **args) {
    return 0;
}

int run_history(char **args) {
    /* run command n, with args = '!n' */
    long num = strtol(*args + 1, NULL, 10);
    
    if(errno == EINVAL) {
        fprintf(stderr, "langursh: conversion error: %d\n", errno);
        return 2;
    } else if (errno == ERANGE) {
        fprintf(stderr, "langursh: value out of range: %s\n", *args);
        return 2;
    }
    
    if(num < 0) {
        int idx = (hist_i + num) % hist_size;
        if(idx < 0) { idx += hist_size; }
        if(-num <= hist_size && idx >= 0) {
            //printf("%ld %d %d run: %s\n", num, hist_i, idx, history[idx]);
            printf("\033[A");
            printf("\33[2K");
            print_prompt();
            printf("%s\n", history[idx]);
            run_line(history[idx]);
        } else {
            fprintf(stderr, "langursh: history index out of bounds: [-1..%d]\n", -hist_size);
        }
    } else if(num < hist_size) {
        //printf("run: %s\n", history[num]);
        printf("\033[A");
        printf("\33[2K");
        print_prompt();
        printf("%s\n", history[num]);
        run_line(history[num]);
    } else {
        fprintf(stderr, "langursh: history index out of bounds: [0..%d]\n", hist_size);
    }
    return 2;
}

int run_print_history(char **args) {
    /* print history */
    int n = 0;
    int i = hist_size == HISTORY_SIZE ? hist_i : 0;
    for(; n < hist_size; i = (i + 1) % hist_size){
        printf("%i    %s\n", i, history[i]);
        n++;
    }
    return 2;
}

int run_cd(char **args) { 
    char *newpwd = args[1];

    // if no argument, move to homedir
    if(!newpwd){
        newpwd = homedir;
    }

    // replace '~' with home directory
    replace_str(newpwd, "~", homedir);

    // change directory
    if(chdir(newpwd) != 0) {
        perror("langursh");
    }
    return 1;
}

int run_help(char **args) {
    printf("Langur Shell (langursh), version %s\n", VERSION);
    printf("\"A good monkey's shell\"\n");
    printf("Copyright © 2019 Toni Sagristà Sellés\n\n");

    printf("Built-in commands:\n");
    for(int i = 0; i < n_builtin; i++) {
        printf("\t%s\n", builtin_cmd[i]);
        printf("\t\t%s\n", builtin_cmd_desc[i]);
    }
    return 1;
}

int run_process(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if(pid == 0) {
        // child
        if(execvp(args[0], args) == -1) {
            fprintf(stderr, "langursh: command or program not found\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) { 
        // error in fork
        perror("langursh");
    } else {
        // parent
        do {
            waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int starts_with(const char *str, const char *pre) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void print_prompt(){
    char cwd[1024];
    char hostname[1024];
    char* username;
    
    /* path */
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("langursh: getcwd() error");
    }
    /* hostname */
    if(gethostname(hostname, sizeof(hostname)) == -1) {
        perror("langursh: gethostname() error");
    }
    /* username */
    if((username = getlogin()) == NULL) {
        perror("langursh: getlogin() error");
    }
    
    printf("%s@%s:%s $ ", username, hostname, cwd);
}
