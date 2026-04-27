#include "food.h"
#include "snake.h"
#include "raylib.h"

void FoodSpawn(Food *food, const Snake *snake, int grid_size, int screen_width, int screen_height) {
    int cols = screen_width / grid_size;
    int rows = screen_height / grid_size;

    int x = 0;
    int y = 0;
    int attempts = 0;

    do {
        x = GetRandomValue(0, cols - 1);
        y = GetRandomValue(0, rows - 1);
        attempts++;
    } while (SnakeOccupiesCell(snake, x, y) && attempts < 500);

    food->x = x;
    food->y = y;
}

void FoodDraw(const Food *food, int grid_size) {
    int px = food->x * grid_size;
    int py = food->y * grid_size;
    DrawRectangle(px, py, grid_size, grid_size, (Color){220, 80, 80, 255});
}
