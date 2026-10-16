#ifndef TETRIS_H
#define TETRIS_H
#include <stdbool.h>
#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 20
typedef struct {
    unsigned char board[TETRIS_HEIGHT][TETRIS_WIDTH];
    int type, next, rotation, x, y, score, lines;
    bool over;
} Tetris;
void tetris_init(Tetris *g, unsigned seed);
bool tetris_cell(int type, int rotation, int x, int y);
bool tetris_move(Tetris *g, int dx, int dy);
bool tetris_rotate(Tetris *g);
void tetris_tick(Tetris *g);
void tetris_drop(Tetris *g);
int tetris_clear_lines(Tetris *g);
#endif
