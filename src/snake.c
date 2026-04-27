#include "snake.h"
#include "raylib.h"
#include <stdlib.h>

static void SnakeEnsureCapacity(Snake *snake, int needed) {
    if (needed <= snake->capacity) {
        return;
    }

    int new_capacity = snake->capacity * 2;
    if (new_capacity < needed) {
        new_capacity = needed;
    }

    SnakeSegment *new_segments = (SnakeSegment *)MemAlloc(sizeof(SnakeSegment) * new_capacity);
    for (int i = 0; i < snake->length; i++) {
        new_segments[i] = snake->segments[i];
    }
    MemFree(snake->segments);
    snake->segments = new_segments;
    snake->capacity = new_capacity;
}

void SnakeInit(Snake *snake, int grid_size, int screen_width, int screen_height) {
    snake->capacity = 64;
    snake->length = 3;
    snake->segments = (SnakeSegment *)MemAlloc(sizeof(SnakeSegment) * snake->capacity);

    int start_x = (screen_width / grid_size) / 2;
    int start_y = (screen_height / grid_size) / 2;

    for (int i = 0; i < snake->length; i++) {
        snake->segments[i].x = start_x - i;
        snake->segments[i].y = start_y;
    }

    snake->dir_x = 1;
    snake->dir_y = 0;
}

void SnakeUnload(Snake *snake) {
    if (snake->segments) {
        MemFree(snake->segments);
        snake->segments = NULL;
    }
    snake->length = 0;
    snake->capacity = 0;
}

void SnakeSetDirection(Snake *snake, int dir_x, int dir_y) {
    // Prevent reversing into itself.
    if (snake->length > 1 && dir_x == -snake->dir_x && dir_y == -snake->dir_y) {
        return;
    }
    snake->dir_x = dir_x;
    snake->dir_y = dir_y;
}

void SnakeMove(Snake *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->segments[i] = snake->segments[i - 1];
    }

    snake->segments[0].x += snake->dir_x;
    snake->segments[0].y += snake->dir_y;
}

void SnakeGrow(Snake *snake) {
    SnakeEnsureCapacity(snake, snake->length + 1);
    snake->segments[snake->length] = snake->segments[snake->length - 1];
    snake->length += 1;
}

int SnakeCheckWallCollision(const Snake *snake, int grid_size, int screen_width, int screen_height) {
    int cols = screen_width / grid_size;
    int rows = screen_height / grid_size;

    int x = snake->segments[0].x;
    int y = snake->segments[0].y;

    return (x < 0 || y < 0 || x >= cols || y >= rows);
}

int SnakeCheckSelfCollision(const Snake *snake) {
    int head_x = snake->segments[0].x;
    int head_y = snake->segments[0].y;

    for (int i = 1; i < snake->length; i++) {
        if (snake->segments[i].x == head_x && snake->segments[i].y == head_y) {
            return 1;
        }
    }

    return 0;
}

int SnakeCheckFoodCollision(const Snake *snake, const Food *food) {
    return snake->segments[0].x == food->x && snake->segments[0].y == food->y;
}

int SnakeOccupiesCell(const Snake *snake, int x, int y) {
    for (int i = 0; i < snake->length; i++) {
        if (snake->segments[i].x == x && snake->segments[i].y == y) {
            return 1;
        }
    }
    return 0;
}

void SnakeDraw(const Snake *snake, int grid_size) {
    for (int i = 0; i < snake->length; i++) {
        int px = snake->segments[i].x * grid_size;
        int py = snake->segments[i].y * grid_size;
        Color color = (i == 0) ? (Color){80, 220, 120, 255} : (Color){50, 180, 90, 255};
        DrawRectangle(px, py, grid_size, grid_size, color);
    }
}
