#pragma once
#include "game.h"
#include <ncurses.h>

// Color pair constants
constexpr int COLOR_PAIR_I      = 1; // cyan
constexpr int COLOR_PAIR_O      = 2; // yellow
constexpr int COLOR_PAIR_T      = 3; // magenta
constexpr int COLOR_PAIR_S      = 4; // green
constexpr int COLOR_PAIR_Z      = 5; // red
constexpr int COLOR_PAIR_J      = 6; // blue
constexpr int COLOR_PAIR_L      = 7; // white (orange approximation)
constexpr int COLOR_PAIR_GHOST  = 8; // dark/dim
constexpr int COLOR_PAIR_BORDER = 9; // white border
constexpr int COLOR_PAIR_UI     = 10; // white on black

class Renderer {
public:
    Renderer();
    ~Renderer();

    void draw(const Game& game);

private:
    WINDOW* mainWin_{ nullptr };
    WINDOW* boardWin_{ nullptr };
    WINDOW* sideWin_{ nullptr };

    void initColors();
    void drawBoard(const Game& game);
    void drawSidebar(const Game& game);
    void drawMiniPiece(WINDOW* win, int startRow, int startCol, TetrominoType type);
    int  colorPairFor(TetrominoType t) const;
};
