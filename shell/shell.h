#ifndef SHELL
#define SHELL

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10

void command_line_parse(char *input, char *command, char **arguments);
void command_line_execute(char *command, char **arguments);

#endif
