#include "game.h"
#include "raylib.h"

int main(void) {
    const int screen_width = 800;
    const int screen_height = 600;

    InitWindow(screen_width, screen_height, "Snake - Raylib");
    SetTargetFPS(60);

    Game game;
    GameInit(&game, screen_width, screen_height);

    while (!WindowShouldClose()) {
        GameUpdate(&game);

        BeginDrawing();
        ClearBackground((Color){20, 20, 20, 255});
        GameDraw(&game);
        EndDrawing();
    }

    GameUnload(&game);
    CloseWindow();
    return 0;
}
