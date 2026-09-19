#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "amnesic.h"
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

int launch_in_standalone_terminal(int argc, char *argv[]) {
    static const char *terms[] = {
        "x-terminal-emulator",
        "gnome-terminal",
        "konsole",
        "xfce4-terminal",
        "kitty",
        "alacritty",
        "xterm"
    };
    size_t num_terms = sizeof(terms) / sizeof(terms[0]);

    const char *found_term = NULL;
    for (size_t i = 0; i < num_terms; i++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", terms[i]);
        if (system(cmd) == 0) {
            found_term = terms[i];
            break;
        }
    }

    if (!found_term) {
        fprintf(stderr, "Error: No suitable terminal emulator found (x-terminal-emulator, gnome-terminal, konsole, xterm, etc.).\n");
        return AMNESIC_ERR_IO;
    }

    char exec_cmd[4096];
    size_t off = snprintf(exec_cmd, sizeof(exec_cmd), "\"%s\" --child-terminal", argv[0]);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--terminal") == 0) {
            continue;
        }
        off += snprintf(exec_cmd + off, sizeof(exec_cmd) - off, " \"%s\"", argv[i]);
    }

    char term_cmd[8192];
    if (strcmp(found_term, "gnome-terminal") == 0) {
        snprintf(term_cmd, sizeof(term_cmd), "gnome-terminal -- bash -c '%s'", exec_cmd);
    } else if (strcmp(found_term, "konsole") == 0) {
        snprintf(term_cmd, sizeof(term_cmd), "konsole -e bash -c '%s'", exec_cmd);
    } else if (strcmp(found_term, "xfce4-terminal") == 0) {
        snprintf(term_cmd, sizeof(term_cmd), "xfce4-terminal -e \"bash -c '%s'\"", exec_cmd);
    } else {
        snprintf(term_cmd, sizeof(term_cmd), "%s -e bash -c '%s'", found_term, exec_cmd);
    }

    int res = system(term_cmd);
    return (res == 0) ? AMNESIC_SUCCESS : AMNESIC_ERR_IO;
}
