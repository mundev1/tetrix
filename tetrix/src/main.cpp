#include "game.h"
#include "renderer.h"
#include <ncurses.h>
#include <chrono>
#include <thread>

int main() {
    Game game;
    Renderer renderer;

    while (true) {
        // Handle input
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        if (ch != ERR) {
            game.handleInput(ch);
        }

        // Update game logic (gravity)
        game.update();

        // Render
        renderer.draw(game);

        // ~60 FPS cap
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
