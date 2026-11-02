#include "renderer.h"
#include <cstring>
#include <string>

// Board cell width in terminal columns (2 chars per cell = "██")
constexpr int CELL_W = 2;

// Layout constants
constexpr int BOARD_DISP_W  = BOARD_WIDTH * CELL_W + 2;  // +2 for borders
constexpr int BOARD_DISP_H  = BOARD_HEIGHT + 2;           // +2 for borders
constexpr int SIDE_W        = 20;
constexpr int WIN_H         = BOARD_DISP_H + 4;           // extra rows for title + keys
constexpr int WIN_W         = BOARD_DISP_W + SIDE_W + 2;

Renderer::Renderer() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    initColors();

    // Create main window centered on terminal
    int termH, termW;
    getmaxyx(stdscr, termH, termW);
    int startY = std::max(0, (termH - WIN_H) / 2);
    int startX = std::max(0, (termW - WIN_W) / 2);

    mainWin_  = newwin(WIN_H, WIN_W, startY, startX);
    boardWin_ = derwin(mainWin_, BOARD_DISP_H, BOARD_DISP_W, 2, 1);
    sideWin_  = derwin(mainWin_, WIN_H - 2, SIDE_W, 2, BOARD_DISP_W + 2);
}

Renderer::~Renderer() {
    if (sideWin_)  delwin(sideWin_);
    if (boardWin_) delwin(boardWin_);
    if (mainWin_)  delwin(mainWin_);
    endwin();
}

void Renderer::initColors() {
    if (!has_colors()) return;
    start_color();
    use_default_colors();
    // Pair(index, fg, bg)
    init_pair(COLOR_PAIR_I,      COLOR_CYAN,    COLOR_BLACK);
    init_pair(COLOR_PAIR_O,      COLOR_YELLOW,  COLOR_BLACK);
    init_pair(COLOR_PAIR_T,      COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_PAIR_S,      COLOR_GREEN,   COLOR_BLACK);
    init_pair(COLOR_PAIR_Z,      COLOR_RED,     COLOR_BLACK);
    init_pair(COLOR_PAIR_J,      COLOR_BLUE,    COLOR_BLACK);
    init_pair(COLOR_PAIR_L,      COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLOR_PAIR_GHOST,  COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLOR_PAIR_BORDER, COLOR_WHITE,   COLOR_BLACK);
    init_pair(COLOR_PAIR_UI,     COLOR_WHITE,   COLOR_BLACK);
}

int Renderer::colorPairFor(TetrominoType t) const {
    return static_cast<int>(t); // 1=I..7=L, 0=NONE
}

void Renderer::draw(const Game& game) {
    werase(mainWin_);

    // Title bar
    wattron(mainWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_UI));
    mvwprintw(mainWin_, 0, 2, " TETRIX ");
    wattroff(mainWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_UI));

    // Main border
    wattron(mainWin_, COLOR_PAIR(COLOR_PAIR_BORDER));
    box(mainWin_, 0, 0);
    wattroff(mainWin_, COLOR_PAIR(COLOR_PAIR_BORDER));

    drawBoard(game);
    drawSidebar(game);

    // Controls bar at bottom
    int bottomRow = WIN_H - 2;
    wattron(mainWin_, COLOR_PAIR(COLOR_PAIR_UI));
    mvwprintw(mainWin_, bottomRow,     2, "Arrows:Move  Up:Rotate  Z:CCW");
    mvwprintw(mainWin_, bottomRow + 1, 2, "Space:Drop  C/H:Hold  P:Pause  R:Restart");
    wattroff(mainWin_, COLOR_PAIR(COLOR_PAIR_UI));

    wrefresh(mainWin_);
}

void Renderer::drawBoard(const Game& game) {
    werase(boardWin_);
    // Board border
    wattron(boardWin_, COLOR_PAIR(COLOR_PAIR_BORDER));
    box(boardWin_, 0, 0);
    wattroff(boardWin_, COLOR_PAIR(COLOR_PAIR_BORDER));

    const auto& board  = game.getBoard();
    const auto& cur    = game.getCurrent();
    const auto& ghost  = game.getGhost();
    const auto& curShape   = cur.currentShape();
    const auto& ghostShape = ghost.currentShape();

    // Draw ghost piece first (behind current)
    if (game.getState() == GameState::PLAYING) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (!ghostShape[row][col]) continue;
                int bx = ghost.x + col;
                int by = ghost.y + row;
                if (by < 0 || by >= BOARD_HEIGHT) continue;
                if (bx < 0 || bx >= BOARD_WIDTH)  continue;
                wattron(boardWin_, COLOR_PAIR(COLOR_PAIR_GHOST) | A_DIM);
                mvwprintw(boardWin_, by + 1, bx * CELL_W + 1, "░░");
                wattroff(boardWin_, COLOR_PAIR(COLOR_PAIR_GHOST) | A_DIM);
            }
        }
    }

    // Draw board cells
    for (int row = 0; row < BOARD_HEIGHT; ++row) {
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            TetrominoType cell = board[row][col];
            if (cell != TetrominoType::NONE) {
                int cp = colorPairFor(cell);
                wattron(boardWin_, COLOR_PAIR(cp) | A_BOLD);
                mvwprintw(boardWin_, row + 1, col * CELL_W + 1, "██");
                wattroff(boardWin_, COLOR_PAIR(cp) | A_BOLD);
            }
        }
    }

    // Draw current piece
    if (game.getState() != GameState::GAME_OVER) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (!curShape[row][col]) continue;
                int bx = cur.x + col;
                int by = cur.y + row;
                if (by < 0 || by >= BOARD_HEIGHT) continue;
                if (bx < 0 || bx >= BOARD_WIDTH)  continue;
                int cp = colorPairFor(cur.type);
                wattron(boardWin_, COLOR_PAIR(cp) | A_BOLD);
                mvwprintw(boardWin_, by + 1, bx * CELL_W + 1, "██");
                wattroff(boardWin_, COLOR_PAIR(cp) | A_BOLD);
            }
        }
    }

    // Game over / paused overlay
    if (game.getState() == GameState::GAME_OVER) {
        wattron(boardWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_Z));
        mvwprintw(boardWin_, BOARD_HEIGHT / 2,     2, "  GAME OVER!  ");
        mvwprintw(boardWin_, BOARD_HEIGHT / 2 + 1, 2, " Press R to   ");
        mvwprintw(boardWin_, BOARD_HEIGHT / 2 + 2, 2, "  restart     ");
        wattroff(boardWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_Z));
    } else if (game.getState() == GameState::PAUSED) {
        wattron(boardWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_O));
        mvwprintw(boardWin_, BOARD_HEIGHT / 2,     3, "  PAUSED  ");
        mvwprintw(boardWin_, BOARD_HEIGHT / 2 + 1, 3, " P:Resume ");
        wattroff(boardWin_, A_BOLD | COLOR_PAIR(COLOR_PAIR_O));
    }

    wnoutrefresh(boardWin_);
}

void Renderer::drawMiniPiece(WINDOW* win, int startRow, int startCol, TetrominoType type) {
    if (type == TetrominoType::NONE) return;
    const auto& shape = Tetromino::shapeOf(type, Rotation::R0);
    int cp = colorPairFor(type);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (!shape[row][col]) continue;
            wattron(win, COLOR_PAIR(cp) | A_BOLD);
            mvwprintw(win, startRow + row, startCol + col * CELL_W, "██");
            wattroff(win, COLOR_PAIR(cp) | A_BOLD);
        }
    }
}

void Renderer::drawSidebar(const Game& game) {
    werase(sideWin_);
    wattron(sideWin_, COLOR_PAIR(COLOR_PAIR_UI));

    // HOLD section
    mvwprintw(sideWin_, 0, 0, "HOLD:");
    drawMiniPiece(sideWin_, 1, 0, game.getHold());

    // NEXT section
    mvwprintw(sideWin_, 6, 0, "NEXT:");
    drawMiniPiece(sideWin_, 7, 0, game.getNext());

    // Stats
    mvwprintw(sideWin_, 12, 0, "Score:");
    mvwprintw(sideWin_, 13, 0, "%ld", game.getScore());
    mvwprintw(sideWin_, 15, 0, "Level:");
    mvwprintw(sideWin_, 16, 0, "%d", game.getLevel());
    mvwprintw(sideWin_, 18, 0, "Lines:");
    mvwprintw(sideWin_, 19, 0, "%d", game.getLines());

    wattroff(sideWin_, COLOR_PAIR(COLOR_PAIR_UI));
    wnoutrefresh(sideWin_);
}
