#include "food.h"
#include "snake.h"
#include "raylib.h"

void FoodSpawn(Food *food, const Snake *snakes, int snake_count, const Food *foods, int food_count,
               int grid_size, int screen_width, int screen_height) {
    int cols = screen_width / grid_size;
    int rows = screen_height / grid_size;

    int x = 0;
    int y = 0;
    int attempts = 0;
    int blocked = 0;

    do {
        x = GetRandomValue(0, cols - 1);
        y = GetRandomValue(0, rows - 1);
        attempts++;
        blocked = 0;

        for (int i = 0; i < snake_count; i++) {
            if (SnakeOccupiesCell(&snakes[i], x, y)) {
                blocked = 1;
                break;
            }
        }

        if (!blocked) {
            for (int i = 0; i < food_count; i++) {
                if (foods[i].x == x && foods[i].y == y) {
                    blocked = 1;
                    break;
                }
            }
        }
    } while (blocked && attempts < 500);

    food->x = x;
    food->y = y;
}

void FoodDraw(const Food *food, int grid_size) {
    int px = food->x * grid_size;
    int py = food->y * grid_size;
    DrawRectangle(px, py, grid_size, grid_size, (Color){220, 80, 80, 255});
}
