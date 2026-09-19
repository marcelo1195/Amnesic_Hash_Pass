#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "terminal.h"

static struct termios g_orig_termios;
static bool g_termios_saved = false;

bool disable_terminal_echo(void) {
    if (!isatty(STDIN_FILENO)) {
        return true;
    }
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) != 0) {
        return false;
    }
    g_termios_saved = true;

    struct termios raw = g_orig_termios;
    raw.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        return false;
    }
    return true;
}

void restore_terminal_echo(void) {
    if (g_termios_saved && isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
        g_termios_saved = false;
    }
}

void clear_terminal_screen(void) {
    if (isatty(STDOUT_FILENO) || isatty(STDERR_FILENO)) {
        const char wipe_seq[] = "\033[2J\033[1;1H";
        ssize_t w = write(STDOUT_FILENO, wipe_seq, sizeof(wipe_seq) - 1);
        (void)w;
        fflush(stdout);
    }
}
