#include "tetris.h"
#include <assert.h>
#include <string.h>
int main(void) {
    Tetris g;
    for (int type = 0; type < 7; ++type)
        for (int rotation = 0; rotation < 4; ++rotation) {
            int cells = 0;
            for (int y = 0; y < 4; ++y)
                for (int x = 0; x < 4; ++x)
                    cells += tetris_cell(type, rotation, x, y);
            assert(cells == 4);
        }
    tetris_init(&g, 1);
    memset(g.board[19], 1, TETRIS_WIDTH);
    memset(g.board[18], 1, TETRIS_WIDTH);
    g.board[17][2] = 1;
    assert(tetris_clear_lines(&g) == 2 && g.score == 300 && g.lines == 2);
    assert(g.board[19][2] == 1 && g.board[18][2] == 0);
    for (unsigned seed = 0; seed < 100; ++seed) {
        tetris_init(&g, seed);
        for (int piece = 0; piece < 200 && !g.over; ++piece) {
            for (int i = 0; i < 20; ++i)
                tetris_move(&g, piece % 2 ? 1 : -1, 0);
            for (int i = 0; i < 4; ++i)
                tetris_rotate(&g);
            tetris_drop(&g);
            for (int y = 0; y < TETRIS_HEIGHT; ++y)
                for (int x = 0; x < TETRIS_WIDTH; ++x)
                    assert(g.board[y][x] <= 7);
        }
        assert(g.over);
        Tetris before = g;
        tetris_drop(&g);
        assert(memcmp(&before, &g, sizeof(g)) == 0);
    }
    return 0;
}
