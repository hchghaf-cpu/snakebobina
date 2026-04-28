#ifndef SNAKE_H
#define SNAKE_H

#include "food.h"
#include "raylib.h"

typedef struct SnakeSegment {
    int x;
    int y;
} SnakeSegment;

typedef struct Snake {
    SnakeSegment *segments;
    int length;
    int capacity;
    int dir_x;
    int dir_y;
} Snake;

void SnakeInit(Snake *snake, int grid_size, int screen_width, int screen_height, int start_x, int start_y);
void SnakeUnload(Snake *snake);
void SnakeSetDirection(Snake *snake, int dir_x, int dir_y);
void SnakeMove(Snake *snake);
void SnakeGrow(Snake *snake);
int SnakeCheckWallCollision(const Snake *snake, int grid_size, int screen_width, int screen_height);
int SnakeCheckSelfCollision(const Snake *snake);
int SnakeCheckCollisionWithSnake(const Snake *snake, const Snake *other);
int SnakeCheckFoodCollision(const Snake *snake, const Food *food);
void SnakeDraw(const Snake *snake, int grid_size, Color head_color, Color body_color);
int SnakeOccupiesCell(const Snake *snake, int x, int y);

#endif
