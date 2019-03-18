
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "langursh.h"
#include "parser.h"

char *read_line(void) {
    char *line = malloc(LINE_BUFFSIZE);
    char *linep = line;
    size_t lenmax = LINE_BUFFSIZE;
    size_t len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(!linen) {
                fprintf(stderr, "langursh: allocation error\n");
                free(linep);
                exit(EXIT_FAILURE);
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *--line = '\0';
    return linep;
}

char **tokenize_line(char *line){
    int bufsize = TOKEN_BUFFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "langursh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TOKEN_BUFFSIZE;
            char **tokensp = realloc(tokens, bufsize * sizeof(char*));
            if (!tokensp) {
                fprintf(stderr, "langursh: allocation error\n");
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = tokensp;
        }

        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


void replace_str(char *target, const char *needle, const char *replacement)
{
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}
