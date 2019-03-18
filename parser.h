#define LINE_BUFFSIZE       2048
#define TOKEN_BUFFSIZE      128
#define TOKEN_DELIM         " \t\r\n\a"


/* reads a line from stdin */
char *read_line(void);

/* tokenizes the given string */
char **tokenize_line(char *line);

/* replaces all occurrences of orig with rep in str */
void replace_str(char *target, const char *needle, const char *replacement);
