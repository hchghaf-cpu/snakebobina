#ifndef GAME_H
#define GAME_H

#include "food.h"
#include "raylib.h"
#include "snake.h"

#define MAX_USERS 16
#define MAX_NAME 16
#define MAX_SNAKES 5
#define MAX_FOODS 6

typedef enum GameState {
    GAME_STATE_INTRO,
    GAME_STATE_AUTH,
    GAME_STATE_PLAY,
    GAME_STATE_GAME_OVER
} GameState;

typedef struct User {
    char name[MAX_NAME];
} User;

typedef struct Game {
    int screen_width;
    int screen_height;
    int grid_size;
    int score;
    int is_paused;
    int request_exit;
    int level;
    float move_timer;
    float move_interval;
    float base_interval;
    float min_interval;
    GameState state;
    int auth_is_register;
    char username[MAX_NAME];
    int username_len;
    char auth_message[64];
    float auth_message_timer;
    User users[MAX_USERS];
    int user_count;
    int current_user;
    int team_index;
    Snake snakes[MAX_SNAKES];
    int snake_count;
    Food foods[MAX_FOODS];
    int food_count;
    Sound eat_sound;
    Sound game_over_sound;
} Game;

void GameInit(Game *game, int screen_width, int screen_height);
void GameUpdate(Game *game);
void GameDraw(const Game *game);
void GameUnload(Game *game);

#endif
