#ifndef GAME_H
#define GAME_H

#include "food.h"
#include "raylib.h"
#include "snake.h"

typedef struct Game {
    int screen_width;
    int screen_height;
    int grid_size;
    int score;
    int is_game_over;
    int is_paused;
    int is_intro;
    float move_timer;
    float move_interval;
    float base_interval;
    float min_interval;
    Snake snake;
    Food food;
    Sound eat_sound;
    Sound game_over_sound;
} Game;

void GameInit(Game *game, int screen_width, int screen_height);
void GameUpdate(Game *game);
void GameDraw(const Game *game);
void GameUnload(Game *game);

#endif
