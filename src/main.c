#define _POSIX_C_SOURCE 200809L
#include "tetris.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static volatile sig_atomic_t interrupted = 0;
static void on_signal(int signum) {
    (void)signum;
    interrupted = 1;
}
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
static HANDLE output;
static DWORD original_mode;
static void restore(void) {
    SetConsoleMode(output, original_mode);
    fputs("\x1b[?25h\n", stdout);
}
static int input(void) {
    if (!_kbhit())
        return -1;
    int key = _getch();
    if (key == 0 || key == 224) {
        switch (_getch()) {
        case 75:
            return 'a';
        case 77:
            return 'd';
        case 72:
            return 'w';
        case 80:
            return 's';
        default:
            return -1;
        }
    }
    return key;
}
static void wait_frame(void) { Sleep(20); }
static unsigned long long now_ms(void) { return GetTickCount64(); }
static bool setup(void) {
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleMode(output, &original_mode))
        return false;
    if (!SetConsoleMode(output, original_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        return false;
    atexit(restore);
    return true;
}
#else
#include <poll.h>
#include <termios.h>
#include <unistd.h>
static struct termios original;
static void restore(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    fputs("\x1b[?25h\n", stdout);
}
static int read_key(void) {
    struct pollfd descriptor = {STDIN_FILENO, POLLIN, 0};
    if (poll(&descriptor, 1, 0) <= 0)
        return -1;
    unsigned char key;
    return read(STDIN_FILENO, &key, 1) == 1 ? key : -1;
}
static int input(void) {
    /* Keep escape parsing state across frames; terminal sequences may arrive in chunks. */
    static int escape = 0;
    int key = read_key();
    if (key < 0)
        return -1;
    if (!escape && key == 27) {
        escape = 1;
        return -1;
    }
    if (escape == 1 && key == '[') {
        escape = 2;
        return -1;
    }
    if (escape) {
        int state = escape;
        escape = 0;
        if (state == 2)
            switch (key) {
            case 'A':
                return 'w';
            case 'B':
                return 's';
            case 'C':
                return 'd';
            case 'D':
                return 'a';
            }
    }
    return key;
}
static void wait_frame(void) {
    const struct timespec delay = {0, 20000000};
    nanosleep(&delay, NULL);
}
static unsigned long long now_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (unsigned long long)now.tv_sec * 1000 + (unsigned long long)now.tv_nsec / 1000000;
}
static bool setup(void) {
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || tcgetattr(STDIN_FILENO, &original))
        return false;
    struct termios raw = original;
    raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw))
        return false;
    atexit(restore);
    return true;
}
#endif
static void draw(const Tetris *g, bool paused) {
    printf("\x1b[HScore: %-8d Lines: %-6d %s\x1b[K\n", g->score, g->lines, paused ? "PAUSED" : "");
    puts("+--------------------+");
    for (int y = 0; y < TETRIS_HEIGHT; ++y) {
        putchar('|');
        for (int x = 0; x < TETRIS_WIDTH; ++x) {
            bool occupied = g->board[y][x] || tetris_cell(g->type, g->rotation, x - g->x, y - g->y);
            fputs(occupied ? "[]" : "  ", stdout);
        }
        puts("|");
    }
    puts("+--------------------+");
    printf("Next: %c\n", "IJLOSTZ"[g->next]);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x)
            fputs(tetris_cell(g->next, 0, x, y) ? "[]" : "  ", stdout);
        puts("\x1b[K");
    }
    puts("Arrows/WASD: move/rotate | Space: drop | P: pause | Q: quit");
    fflush(stdout);
}
int main(void) {
    if (!setup()) {
        fputs("Run in an interactive terminal (Windows 10+ or POSIX).\n", stderr);
        return 1;
    }
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    Tetris game;
    tetris_init(&game, (unsigned)time(NULL));
    fputs("\x1b[2J\x1b[?25l", stdout);
    bool paused = false;
    unsigned long long last = now_ms();
    while (!game.over && !interrupted) {
        int key = input();
        if (key == 'q' || key == 'Q')
            break;
        if (key == 'p' || key == 'P') {
            paused = !paused;
            last = now_ms();
        }
        if (!paused) {
            switch (key) {
            case 'a':
                tetris_move(&game, -1, 0);
                break;
            case 'd':
                tetris_move(&game, 1, 0);
                break;
            case 's':
                tetris_tick(&game);
                break;
            case 'w':
                tetris_rotate(&game);
                break;
            case ' ':
                tetris_drop(&game);
                break;
            }
            if (now_ms() - last >= 500) {
                tetris_tick(&game);
                last = now_ms();
            }
        }
        draw(&game, paused);
        wait_frame();
    }
    printf("\n%s Final score: %d\n", game.over ? "Game over." : "Game ended.", game.score);
    return 0;
}
