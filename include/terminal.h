#ifndef AMNESIC_TERMINAL_H
#define AMNESIC_TERMINAL_H

#include <stdbool.h>

bool disable_terminal_echo(void);
void restore_terminal_echo(void);
void clear_terminal_screen(void);
int launch_in_standalone_terminal(int argc, char *argv[]);

#endif
