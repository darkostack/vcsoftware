#ifndef SHELL_H_
#define SHELL_H_

#include "sys/process.h"

struct shell_command
{
    struct shell_command *next;
    char *command;
    char *description;
    struct process *process;
    struct shell_command *child;
};

void shell_init(void);
void shell_start(void);
void shell_input(char *commandline, int commandline_len);
void shell_stop(void);
void shell_quit(void);
void shell_prompt(char *prompt);
void shell_default_output(const char *data1, int size1, const char *data2, int size2);
void shell_exit(void);

#define SHELL_COMMAND(name, command, description, process) \
    static struct shell_command name = { NULL, command, description, process, NULL }

void shell_output(struct shell_command *c, void *data1, int size1, const void *data2, int size2);
void shell_output_str(struct shell_command *c, char *str1, const char *str2);
void shell_register_command(struct shell_command *c);
void shell_unregister_command(struct shell_command *c);
int shell_start_command(char *commandline, int commandline_len, struct shell_command *child, struct process **started_process);

unsigned long shell_strtolong(const char *str, const char **retstr);

unsigned long shell_time(void);
void shell_set_time(unsigned long seconds);

enum shell_retval {
  SHELL_FOREGROUND,
  SHELL_BACKGROUND,
  SHELL_NOTHING,
};

extern int shell_event_input;

struct shell_input {
  char *data1;
  const char *data2;
  int len1, len2;
};

#endif /* SHELL_H_ */
