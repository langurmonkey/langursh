#define VERSION         "0.0.1-alpha"
#define HISTORY_SIZE    8

#define max(a,b) \
       ({ typeof (a) _a = (a); \
           typeof (b) _b = (b); \
         _a > _b ? _a : _b; })
#define min(a,b) \
       ({ typeof (a) _a = (a); \
           typeof (b) _b = (b); \
         _a < _b ? _a : _b; })

/* prints the prompt line */
void print_prompt();

/* initialises the shell */
void init();

/* adds line to history */
void add_history(char *line);

/* acts on the given tokens */
int act(char **tokens);

/* runs built-in cd command */
int run_cd(char **args);

/* prints the history */
int run_print_history(char **args);

/* runs a history command */
int run_history(char **args);

/* runs built-in exit command */
int run_exit(char **args);

/* runs built-in help command */
int run_help(char **args);

/* runs a process */
int run_process(char **args);

/* runs a line as if it came from stdin */
int run_line(char *line);

/* checks if str starts with pre */
int starts_with(const char *str, const char *pre);

/* creates a copy of the givne string */
char *str_copy(char *str); 

/* frees all memory */
void dispose();
