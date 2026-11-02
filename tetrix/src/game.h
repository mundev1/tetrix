#pragma once
#include "tetromino.h"
#include <array>
#include <vector>
#include <random>
#include <chrono>

constexpr int BOARD_WIDTH  = 10;
constexpr int BOARD_HEIGHT = 20;

enum class GameState {
    PLAYING,
    PAUSED,
    GAME_OVER
};

class Game {
public:
    Game();

    void update();        // Called each frame; handles gravity and input
    void handleInput(int ch);
    void restart();

    // Accessors for renderer
    const std::array<std::array<TetrominoType, BOARD_WIDTH>, BOARD_HEIGHT>& getBoard() const { return board_; }
    const Tetromino& getCurrent() const { return current_; }
    const Tetromino& getGhost()   const { return ghost_; }
    TetrominoType     getHold()   const { return holdType_; }
    TetrominoType     getNext()   const { return nextType_; }
    long              getScore()  const { return score_; }
    int               getLevel()  const { return level_; }
    int               getLines()  const { return lines_; }
    GameState         getState()  const { return state_; }
    bool              canHold()   const { return canHold_; }

private:
    // Board: 0 = empty, else TetrominoType
    std::array<std::array<TetrominoType, BOARD_WIDTH>, BOARD_HEIGHT> board_{};

    Tetromino current_{};
    Tetromino ghost_{};
    TetrominoType holdType_{ TetrominoType::NONE };
    TetrominoType nextType_{ TetrominoType::NONE };

    long  score_{ 0 };
    int   level_{ 1 };
    int   lines_{ 0 };
    GameState state_{ GameState::PLAYING };
    bool  canHold_{ true };

    // Timing
    std::chrono::steady_clock::time_point lastTick_;
    int gravityMs_{ 1000 }; // milliseconds per drop

    // RNG for bag system
    std::mt19937 rng_;
    std::vector<TetrominoType> bag_;

    // Internal helpers
    TetrominoType drawNext();
    void fillBag();
    void spawnPiece();
    bool isValid(const Tetromino& t) const;
    bool tryMove(Tetromino& t, int dx, int dy) const;
    bool tryRotate(Tetromino& t, bool clockwise);
    void lockPiece();
    void clearLines();
    void updateGhost();
    void hardDrop();
    void softDrop();
    void hold();
    int  gravityForLevel(int lvl) const;
    bool isGameOver() const;
    Tetromino makeTetromino(TetrominoType type) const;
};
