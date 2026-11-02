#pragma once
#include <array>
#include <vector>

// Tetromino types
enum class TetrominoType {
    NONE = 0,
    I,
    O,
    T,
    S,
    Z,
    J,
    L,
    COUNT
};

// Rotation states
enum class Rotation {
    R0 = 0,
    R1,
    R2,
    R3,
    COUNT
};

inline Rotation rotateClockwise(Rotation r) {
    return static_cast<Rotation>((static_cast<int>(r) + 1) % 4);
}

inline Rotation rotateCounter(Rotation r) {
    return static_cast<Rotation>((static_cast<int>(r) + 3) % 4);
}

// A 4x4 bitmask representing one rotation of a tetromino
using Shape = std::array<std::array<int, 4>, 4>;

struct Tetromino {
    TetrominoType type;
    int x;   // board column of top-left of bounding box
    int y;   // board row of top-left of bounding box
    Rotation rotation;

    static const std::array<std::array<Shape, 4>, 8> SHAPES; // indexed by TetrominoType
    static const std::array<int, 8> COLORS; // ncurses color pair index

    const Shape& currentShape() const;
    static const Shape& shapeOf(TetrominoType t, Rotation r);
};

// SRS wall kick offsets: for each piece type (JLSTZ vs I vs O) and rotation transition
// Returns list of (dx, dy) offsets to try
struct WallKick {
    static std::vector<std::pair<int,int>> getOffsets(TetrominoType type, Rotation from, Rotation to);
};
