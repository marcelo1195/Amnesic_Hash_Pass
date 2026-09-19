#ifndef AMNESIC_TERMINAL_H
#define AMNESIC_TERMINAL_H

#include <stdbool.h>

bool disable_terminal_echo(void);
void restore_terminal_echo(void);
void clear_terminal_screen(void);

#endif
