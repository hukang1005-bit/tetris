#include "tetris.h"
#include <stdlib.h>
#include <string.h>
/* I, J, L, O, S, T, Z. Rotation is computed in a bounded 4x4 local frame. */
static const unsigned short shapes[] = {0x00f0, 0x0071, 0x0074, 0x0066, 0x0036, 0x0072, 0x0063};
bool tetris_cell(int type, int rotation, int x, int y) {
    if (type < 0 || type >= 7 || x < 0 || y < 0 || x >= 4 || y >= 4)
        return false;
    if (type == 3)
        rotation = 0;
    for (int i = 0; i < (rotation & 3); ++i) {
        int temp = x;
        x = y;
        y = 3 - temp;
    }
    return (shapes[type] & (1u << (y * 4 + x))) != 0;
}
static bool fits(const Tetris *g, int x, int y, int rotation) {
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) {
            if (!tetris_cell(g->type, rotation, col, row))
                continue;
            int bx = x + col, by = y + row;
            if (bx < 0 || bx >= TETRIS_WIDTH || by < 0 || by >= TETRIS_HEIGHT || g->board[by][bx])
                return false;
        }
    return true;
}
static void spawn(Tetris *g) {
    g->type = g->next;
    g->next = rand() % 7;
    g->rotation = 0;
    g->x = 3;
    g->y = 0;
    g->over = !fits(g, g->x, g->y, g->rotation);
}
void tetris_init(Tetris *g, unsigned seed) {
    memset(g, 0, sizeof(*g));
    srand(seed);
    g->next = rand() % 7;
    spawn(g);
}
bool tetris_move(Tetris *g, int dx, int dy) {
    /* Callers move one cell at a time; rejecting arbitrary deltas avoids overflow. */
    if (g->over || dx < -1 || dx > 1 || dy < -1 || dy > 1 ||
        !fits(g, g->x + dx, g->y + dy, g->rotation))
        return false;
    g->x += dx;
    g->y += dy;
    return true;
}
bool tetris_rotate(Tetris *g) {
    if (g->over)
        return false;
    int rotation = (g->rotation + 1) % 4;
    const int offsets[] = {0, -1, 1, -2, 2};
    for (int i = 0; i < 5; ++i) {
        if (fits(g, g->x + offsets[i], g->y, rotation)) {
            g->x += offsets[i];
            g->rotation = rotation;
            return true;
        }
    }
    return false;
}
int tetris_clear_lines(Tetris *g) {
    int destination = TETRIS_HEIGHT - 1, cleared = 0;
    for (int row = TETRIS_HEIGHT - 1; row >= 0; --row) {
        bool full = true;
        for (int col = 0; col < TETRIS_WIDTH; ++col)
            if (!g->board[row][col])
                full = false;
        if (full) {
            ++cleared;
            continue;
        }
        if (destination != row)
            memcpy(g->board[destination], g->board[row], TETRIS_WIDTH);
        --destination;
    }
    while (destination >= 0)
        memset(g->board[destination--], 0, TETRIS_WIDTH);
    static const int points[] = {0, 100, 300, 500, 800};
    g->lines += cleared;
    g->score += points[cleared > 4 ? 4 : cleared];
    return cleared;
}
void tetris_tick(Tetris *g) {
    if (g->over || tetris_move(g, 0, 1))
        return;
    if (!fits(g, g->x, g->y, g->rotation)) {
        g->over = true;
        return;
    }
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            if (tetris_cell(g->type, g->rotation, x, y))
                g->board[g->y + y][g->x + x] = (unsigned char)(g->type + 1);
    tetris_clear_lines(g);
    spawn(g);
}
void tetris_drop(Tetris *g) {
    if (g->over)
        return;
    while (tetris_move(g, 0, 1))
        ++g->score;
    tetris_tick(g);
}
