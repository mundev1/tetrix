#include "game.h"
#include <algorithm>
#include <numeric>
#include <ncurses.h>

Game::Game() : rng_(std::random_device{}()) {
    fillBag();
    nextType_ = drawNext();
    spawnPiece();
    lastTick_ = std::chrono::steady_clock::now();
}

void Game::restart() {
    board_ = {};
    holdType_ = TetrominoType::NONE;
    score_ = 0;
    level_ = 1;
    lines_ = 0;
    canHold_ = true;
    state_ = GameState::PLAYING;
    bag_.clear();
    fillBag();
    nextType_ = drawNext();
    spawnPiece();
    lastTick_ = std::chrono::steady_clock::now();
}

TetrominoType Game::drawNext() {
    if (bag_.empty()) fillBag();
    TetrominoType t = bag_.back();
    bag_.pop_back();
    return t;
}

void Game::fillBag() {
    std::vector<TetrominoType> newBag = {
        TetrominoType::I, TetrominoType::O, TetrominoType::T,
        TetrominoType::S, TetrominoType::Z, TetrominoType::J,
        TetrominoType::L
    };
    std::shuffle(newBag.begin(), newBag.end(), rng_);
    bag_.insert(bag_.end(), newBag.begin(), newBag.end());
}

Tetromino Game::makeTetromino(TetrominoType type) const {
    Tetromino t;
    t.type = type;
    t.rotation = Rotation::R0;
    t.x = 3; // center of 10-wide board
    t.y = 0;
    return t;
}

void Game::spawnPiece() {
    current_ = makeTetromino(nextType_);
    nextType_ = drawNext();
    canHold_ = true;
    updateGhost();
    if (!isValid(current_)) {
        state_ = GameState::GAME_OVER;
    }
}

bool Game::isValid(const Tetromino& t) const {
    const auto& shape = t.currentShape();
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (!shape[row][col]) continue;
            int bx = t.x + col;
            int by = t.y + row;
            if (bx < 0 || bx >= BOARD_WIDTH) return false;
            if (by >= BOARD_HEIGHT) return false;
            if (by < 0) continue; // allow above board
            if (board_[by][bx] != TetrominoType::NONE) return false;
        }
    }
    return true;
}

bool Game::tryMove(Tetromino& t, int dx, int dy) const {
    Tetromino tmp = t;
    tmp.x += dx;
    tmp.y += dy;
    if (isValid(tmp)) {
        t = tmp;
        return true;
    }
    return false;
}

bool Game::tryRotate(Tetromino& t, bool clockwise) {
    Rotation newRot = clockwise ? rotateClockwise(t.rotation) : rotateCounter(t.rotation);
    auto offsets = WallKick::getOffsets(t.type, t.rotation, newRot);
    for (auto [dx, dy] : offsets) {
        Tetromino tmp = t;
        tmp.rotation = newRot;
        tmp.x += dx;
        tmp.y -= dy; // SRS uses +y = up, but our board uses +y = down
        if (isValid(tmp)) {
            t = tmp;
            return true;
        }
    }
    return false;
}

void Game::updateGhost() {
    ghost_ = current_;
    while (tryMove(ghost_, 0, 1)) {}
}

void Game::lockPiece() {
    const auto& shape = current_.currentShape();
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (!shape[row][col]) continue;
            int bx = current_.x + col;
            int by = current_.y + row;
            if (by >= 0 && by < BOARD_HEIGHT && bx >= 0 && bx < BOARD_WIDTH) {
                board_[by][bx] = current_.type;
            }
        }
    }
}

void Game::clearLines() {
    int cleared = 0;
    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool full = true;
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (board_[row][col] == TetrominoType::NONE) { full = false; break; }
        }
        if (full) {
            // Shift everything down
            for (int r = row; r > 0; --r) {
                board_[r] = board_[r - 1];
            }
            board_[0] = {};
            ++cleared;
            ++row; // recheck same row index
        }
    }
    if (cleared > 0) {
        lines_ += cleared;
        int pts = 0;
        switch (cleared) {
            case 1: pts = 100 * level_; break;
            case 2: pts = 300 * level_; break;
            case 3: pts = 500 * level_; break;
            case 4: pts = 800 * level_; break;
            default: pts = 800 * level_ * cleared / 4; break;
        }
        score_ += pts;
        level_ = lines_ / 10 + 1;
        gravityMs_ = gravityForLevel(level_);
    }
}

int Game::gravityForLevel(int lvl) const {
    int ms = 1000 - (lvl - 1) * 80;
    return std::max(ms, 50);
}

void Game::hardDrop() {
    int dropped = 0;
    while (tryMove(current_, 0, 1)) ++dropped;
    score_ += dropped * 2;
    lockPiece();
    clearLines();
    spawnPiece();
}

void Game::softDrop() {
    if (!tryMove(current_, 0, 1)) {
        lockPiece();
        clearLines();
        spawnPiece();
    } else {
        score_ += 1;
        updateGhost();
    }
}

void Game::hold() {
    if (!canHold_) return;
    canHold_ = false;
    if (holdType_ == TetrominoType::NONE) {
        holdType_ = current_.type;
        spawnPiece();
    } else {
        TetrominoType prev = holdType_;
        holdType_ = current_.type;
        current_ = makeTetromino(prev);
        updateGhost();
    }
}

void Game::update() {
    if (state_ != GameState::PLAYING) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick_).count();
    if (elapsed >= gravityMs_) {
        lastTick_ = now;
        if (!tryMove(current_, 0, 1)) {
            lockPiece();
            clearLines();
            spawnPiece();
        } else {
            updateGhost();
        }
    }
}

void Game::handleInput(int ch) {
    if (state_ == GameState::GAME_OVER) {
        if (ch == 'r' || ch == 'R') restart();
        return;
    }

    if (ch == 'p' || ch == 'P') {
        state_ = (state_ == GameState::PAUSED) ? GameState::PLAYING : GameState::PAUSED;
        if (state_ == GameState::PLAYING) lastTick_ = std::chrono::steady_clock::now();
        return;
    }

    if (state_ == GameState::PAUSED) return;

    bool moved = false;
    switch (ch) {
        case KEY_LEFT:
            moved = tryMove(current_, -1, 0);
            break;
        case KEY_RIGHT:
            moved = tryMove(current_, 1, 0);
            break;
        case KEY_DOWN:
            softDrop();
            return;
        case KEY_UP:
            moved = tryRotate(current_, true);
            break;
        case 'z': case 'Z':
            moved = tryRotate(current_, false);
            break;
        case ' ':
            hardDrop();
            return;
        case 'c': case 'C': case 'h': case 'H':
            hold();
            return;
        case 'r': case 'R':
            restart();
            return;
        default:
            return;
    }
    if (moved) updateGhost();
}
