# Tetrix

A classic Tetris-like game written in C++17 using ncurses for terminal rendering.

## Features

- Classic 10×20 board
- All 7 standard tetrominoes (I, O, T, S, Z, J, L) in distinct ANSI colors
- SRS-like wall kicks for rotation
- Soft drop (↓), hard drop (Space)
- Hold piece mechanic (C or H)
- Next piece preview
- Line clearing with Nintendo-style scoring
- Level system (speed increases every 10 lines)
- Ghost piece (shows where piece will land)
- Pause/Resume (P)
- Restart (R)

## Controls

| Key         | Action         |
|-------------|----------------|
| ← →         | Move left/right|
| ↑           | Rotate CW      |
| Z           | Rotate CCW     |
| ↓           | Soft drop      |
| Space       | Hard drop      |
| C / H       | Hold piece     |
| P           | Pause/Resume   |
| R           | Restart        |
| Q           | Quit           |

## Scoring

| Lines Cleared | Points         |
|---------------|----------------|
| 1 (Single)    | 100 × level    |
| 2 (Double)    | 300 × level    |
| 3 (Triple)    | 500 × level    |
| 4 (Tetrix!)   | 800 × level    |

Hard drop also adds 2 points per row dropped, and soft drop adds 1 point per row.

## Build & Run

### Requirements

- macOS or Linux
- g++ with C++17 support
- ncurses (`brew install ncurses` on macOS if needed)

### Build

```bash
make
```

### Run

```bash
./tetrix
```

### Clean

```bash
make clean
```

## Project Structure

```
tetrix/
├── Makefile
├── README.md
└── src/
    ├── main.cpp         # Entry point & game loop
    ├── game.h / game.cpp         # Game logic (board, gravity, scoring)
    ├── tetromino.h / tetromino.cpp  # Tetromino shapes, rotations, wall kicks
    └── renderer.h / renderer.cpp   # ncurses rendering
```
