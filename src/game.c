#include "game.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

static const Color kTeamHeadColors[3] = {
    {80, 220, 120, 255},
    {80, 140, 230, 255},
    {230, 140, 80, 255}
};

static const Color kTeamBodyColors[3] = {
    {50, 180, 90, 255},
    {50, 110, 200, 255},
    {200, 110, 50, 255}
};

static Sound CreateBeepSound(float frequency, float duration) {
    int sample_rate = 22050;
    int sample_count = (int)(duration * sample_rate);
    if (sample_count < 1) {
        sample_count = 1;
    }

    short *samples = (short *)MemAlloc(sizeof(short) * sample_count);
    for (int i = 0; i < sample_count; i++) {
        float t = (float)i / (float)sample_rate;
        float value = sinf(2.0f * PI * frequency * t);
        samples[i] = (short)(value * 32000.0f);
    }

    Wave wave = { 0 };
    wave.frameCount = sample_count;
    wave.sampleRate = sample_rate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static int GameLevelFromScore(int score) {
    if (score < 50) {
        return 1;
    }
    if (score < 70) {
        return 2;
    }
    return 3 + (score - 70) / 20;
}

static void GameUpdateSpeed(Game *game) {
    float interval = game->base_interval - (float)(game->level - 1) * 0.01f;
    if (interval < game->min_interval) {
        interval = game->min_interval;
    }
    game->move_interval = interval;
}

static void GameDrawGrid(const Game *game) {
    Color grid = (Color){30, 30, 30, 255};

    for (int x = 0; x <= game->screen_width; x += game->grid_size) {
        DrawLine(x, 0, x, game->screen_height, grid);
    }

    for (int y = 0; y <= game->screen_height; y += game->grid_size) {
        DrawLine(0, y, game->screen_width, y, grid);
    }
}

static int GameFindUser(const Game *game, const char *name) {
    for (int i = 0; i < game->user_count; i++) {
        if (strcmp(game->users[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void GameSetAuthMessage(Game *game, const char *message) {
    strncpy(game->auth_message, message, sizeof(game->auth_message) - 1);
    game->auth_message[sizeof(game->auth_message) - 1] = '\0';
    game->auth_message_timer = 2.0f;
}

static void GameInitSnakes(Game *game, int count) {
    int cols = game->screen_width / game->grid_size;
    int rows = game->screen_height / game->grid_size;
    int center_x = cols / 2;
    int center_y = rows / 2;

    for (int i = 0; i < count; i++) {
        int offset_x = i * 4;
        int offset_y = i * 2;
        int start_x = center_x - offset_x;
        int start_y = center_y + offset_y;

        if (start_x < 2) {
            start_x = 2 + i;
        }
        if (start_y >= rows - 2) {
            start_y = rows - 3 - i;
        }

        SnakeInit(&game->snakes[i], game->grid_size, game->screen_width, game->screen_height, start_x, start_y);
    }
    game->snake_count = count;
}

static void GameEnsureSnakeCount(Game *game, int target_count) {
    if (target_count <= game->snake_count) {
        return;
    }

    int cols = game->screen_width / game->grid_size;
    int rows = game->screen_height / game->grid_size;
    int center_x = cols / 2;
    int center_y = rows / 2;

    for (int i = game->snake_count; i < target_count; i++) {
        int offset_x = i * 4;
        int offset_y = i * 2;
        int start_x = center_x - offset_x;
        int start_y = center_y - offset_y;

        if (start_x < 2) {
            start_x = 2 + i;
        }
        if (start_y < 2) {
            start_y = 2 + i;
        }

        SnakeInit(&game->snakes[i], game->grid_size, game->screen_width, game->screen_height, start_x, start_y);
    }

    game->snake_count = target_count;
}

static void GameInitFoods(Game *game, int count) {
    for (int i = 0; i < count; i++) {
        FoodSpawn(&game->foods[i], game->snakes, game->snake_count, game->foods, i,
            game->grid_size, game->screen_width, game->screen_height);
    }
    game->food_count = count;
}

static void GameEnsureFoodCount(Game *game, int target_count) {
    if (target_count <= game->food_count) {
        return;
    }

    for (int i = game->food_count; i < target_count; i++) {
        FoodSpawn(&game->foods[i], game->snakes, game->snake_count, game->foods, i,
            game->grid_size, game->screen_width, game->screen_height);
    }

    game->food_count = target_count;
}

static void GameReset(Game *game) {
    for (int i = 0; i < game->snake_count; i++) {
        SnakeUnload(&game->snakes[i]);
    }

    game->snake_count = 0;
    game->food_count = 0;
    game->score = 0;
    game->level = 1;
    game->is_paused = 0;
    game->move_timer = 0.0f;
    GameInitSnakes(game, 1);
    GameInitFoods(game, 1);
    GameUpdateSpeed(game);
}

void GameInit(Game *game, int screen_width, int screen_height) {
    game->screen_width = screen_width;
    game->screen_height = screen_height;
    game->grid_size = 20;
    game->base_interval = 0.30f;
    game->min_interval = 0.20f;
    game->request_exit = 0;
    game->state = GAME_STATE_INTRO;
    game->auth_is_register = 0;
    game->username_len = 0;
    game->username[0] = '\0';
    game->auth_message[0] = '\0';
    game->auth_message_timer = 0.0f;
    game->user_count = 0;
    game->current_user = -1;
    game->team_index = 0;
    game->snake_count = 0;
    game->food_count = 0;

    InitAudioDevice();
    game->eat_sound = CreateBeepSound(640.0f, 0.08f);
    game->game_over_sound = CreateBeepSound(180.0f, 0.18f);

    GameReset(game);
}

static void GameHandleAuthInput(Game *game) {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 && game->username_len < (MAX_NAME - 1)) {
            game->username[game->username_len++] = (char)key;
            game->username[game->username_len] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && game->username_len > 0) {
        game->username[--game->username_len] = '\0';
    }

    if (IsKeyPressed(KEY_FOUR)) {
        game->auth_is_register = !game->auth_is_register;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        game->team_index = (game->team_index + 2) % 3;
    } else if (IsKeyPressed(KEY_RIGHT)) {
        game->team_index = (game->team_index + 1) % 3;
    }

    if (IsKeyPressed(KEY_ENTER) && game->username_len > 0) {
        int index = GameFindUser(game, game->username);
        if (game->auth_is_register) {
            if (index >= 0) {
                GameSetAuthMessage(game, "User already exists");
            } else if (game->user_count >= MAX_USERS) {
                GameSetAuthMessage(game, "User list full");
            } else {
                strncpy(game->users[game->user_count].name, game->username, MAX_NAME - 1);
                game->users[game->user_count].name[MAX_NAME - 1] = '\0';
                game->current_user = game->user_count;
                game->user_count++;
                game->state = GAME_STATE_PLAY;
                GameReset(game);
            }
        } else {
            if (index >= 0) {
                game->current_user = index;
                game->state = GAME_STATE_PLAY;
                GameReset(game);
            } else {
                GameSetAuthMessage(game, "User not found");
            }
        }
    }
}

static void GameUpdateLevel(Game *game) {
    int new_level = GameLevelFromScore(game->score);
    if (new_level <= game->level) {
        return;
    }

    game->level = new_level;

    int target_snakes = game->level;
    int target_foods = game->level;

    if (target_snakes > MAX_SNAKES) {
        target_snakes = MAX_SNAKES;
    }
    if (target_foods > MAX_FOODS) {
        target_foods = MAX_FOODS;
    }

    GameEnsureSnakeCount(game, target_snakes);
    GameEnsureFoodCount(game, target_foods);
    GameUpdateSpeed(game);
}

void GameUpdate(Game *game) {
    if (IsKeyPressed(KEY_THREE)) {
        game->request_exit = 1;
        return;
    }

    if (game->auth_message_timer > 0.0f) {
        game->auth_message_timer -= GetFrameTime();
        if (game->auth_message_timer <= 0.0f) {
            game->auth_message[0] = '\0';
        }
    }

    if (game->state == GAME_STATE_INTRO) {
        if (IsKeyPressed(KEY_ONE)) {
            game->state = GAME_STATE_AUTH;
            game->username_len = 0;
            game->username[0] = '\0';
        }
        return;
    }

    if (game->state == GAME_STATE_AUTH) {
        GameHandleAuthInput(game);
        return;
    }

    if (game->state == GAME_STATE_GAME_OVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            game->state = GAME_STATE_AUTH;
            game->username_len = 0;
            game->username[0] = '\0';
        }
        return;
    }

    if (IsKeyPressed(KEY_P)) {
        game->is_paused = !game->is_paused;
    }

    if (game->is_paused) {
        return;
    }

    if (IsKeyPressed(KEY_UP)) {
        for (int i = 0; i < game->snake_count; i++) {
            SnakeSetDirection(&game->snakes[i], 0, -1);
        }
    } else if (IsKeyPressed(KEY_DOWN)) {
        for (int i = 0; i < game->snake_count; i++) {
            SnakeSetDirection(&game->snakes[i], 0, 1);
        }
    } else if (IsKeyPressed(KEY_LEFT)) {
        for (int i = 0; i < game->snake_count; i++) {
            SnakeSetDirection(&game->snakes[i], -1, 0);
        }
    } else if (IsKeyPressed(KEY_RIGHT)) {
        for (int i = 0; i < game->snake_count; i++) {
            SnakeSetDirection(&game->snakes[i], 1, 0);
        }
    }

    game->move_timer += GetFrameTime();
    if (game->move_timer < game->move_interval) {
        return;
    }
    game->move_timer = 0.0f;

    for (int i = 0; i < game->snake_count; i++) {
        SnakeMove(&game->snakes[i]);
    }

    for (int i = 0; i < game->snake_count; i++) {
        if (SnakeCheckWallCollision(&game->snakes[i], game->grid_size, game->screen_width, game->screen_height) ||
            SnakeCheckSelfCollision(&game->snakes[i])) {
            game->state = GAME_STATE_GAME_OVER;
            PlaySound(game->game_over_sound);
            return;
        }

        for (int j = 0; j < game->snake_count; j++) {
            if (i == j) {
                continue;
            }
            if (SnakeCheckCollisionWithSnake(&game->snakes[i], &game->snakes[j])) {
                game->state = GAME_STATE_GAME_OVER;
                PlaySound(game->game_over_sound);
                return;
            }
        }
    }

    for (int i = 0; i < game->snake_count; i++) {
        for (int f = 0; f < game->food_count; f++) {
            if (SnakeCheckFoodCollision(&game->snakes[i], &game->foods[f])) {
                SnakeGrow(&game->snakes[i]);
                game->score += 10;
                PlaySound(game->eat_sound);
                FoodSpawn(&game->foods[f], game->snakes, game->snake_count, game->foods, game->food_count,
                    game->grid_size, game->screen_width, game->screen_height);
                GameUpdateLevel(game);
            }
        }
    }
}

static void GameDrawIntro(const Game *game) {
    const Color text_color = (Color){230, 230, 230, 255};
    const char *title = "Snake";
    int title_width = MeasureText(title, 48);
    DrawText(title, (game->screen_width - title_width) / 2, 120, 48, text_color);

    const char *hint = "Press 1 to start, 3 to exit";
    int hint_width = MeasureText(hint, 20);
    DrawText(hint, (game->screen_width - hint_width) / 2, 200, 20, text_color);
}

static void GameDrawAuth(const Game *game) {
    const Color text_color = (Color){230, 230, 230, 255};
    const char *title = game->auth_is_register ? "Register" : "Login";
    int title_width = MeasureText(title, 36);
    DrawText(title, (game->screen_width - title_width) / 2, 80, 36, text_color);

    DrawText("Username:", 180, 180, 20, text_color);
    DrawRectangle(300, 175, 240, 28, (Color){40, 40, 40, 255});
    DrawRectangleLines(300, 175, 240, 28, (Color){120, 120, 120, 255});
    DrawText(game->username, 306, 180, 20, text_color);

    DrawText("Team color:", 180, 230, 20, text_color);
    DrawRectangle(300, 225, 60, 28, kTeamHeadColors[game->team_index]);
    DrawRectangleLines(300, 225, 60, 28, (Color){200, 200, 200, 255});

    DrawText("Left/Right to change", 380, 230, 16, text_color);
    DrawText("Press 4 to switch Login/Register", 180, 280, 18, text_color);
    DrawText("Enter to confirm", 180, 310, 18, text_color);
    DrawText("Press 3 to exit", 180, 340, 18, text_color);

    if (game->auth_message[0] != '\0') {
        DrawText(game->auth_message, 180, 380, 18, (Color){230, 120, 120, 255});
    }
}

void GameDraw(const Game *game) {
    const Color text_color = (Color){230, 230, 230, 255};

    GameDrawGrid(game);

    if (game->state == GAME_STATE_INTRO) {
        GameDrawIntro(game);
        return;
    }

    if (game->state == GAME_STATE_AUTH) {
        GameDrawAuth(game);
        return;
    }

    for (int f = 0; f < game->food_count; f++) {
        FoodDraw(&game->foods[f], game->grid_size);
    }

    for (int i = 0; i < game->snake_count; i++) {
        Color head = kTeamHeadColors[game->team_index];
        Color body = kTeamBodyColors[game->team_index];
        SnakeDraw(&game->snakes[i], game->grid_size, head, body);
    }

    DrawText(TextFormat("User: %s", (game->current_user >= 0) ? game->users[game->current_user].name : "-"),
        12, 12, 20, text_color);
    DrawText(TextFormat("Score: %d", game->score), 12, 36, 20, text_color);
    DrawText(TextFormat("Level: %d", game->level), 12, 60, 18, text_color);
    DrawText(TextFormat("Snakes: %d", game->snake_count), 12, 82, 18, text_color);

    if (game->state == GAME_STATE_GAME_OVER) {
        const char *msg = "Game Over - Press Enter";
        int text_width = MeasureText(msg, 28);
        int x = (game->screen_width - text_width) / 2;
        int y = game->screen_height / 2 - 14;
        DrawText(msg, x, y, 28, text_color);
    } else if (game->is_paused) {
        const char *msg = "Paused - Press P";
        int text_width = MeasureText(msg, 28);
        int x = (game->screen_width - text_width) / 2;
        int y = game->screen_height / 2 - 14;
        DrawText(msg, x, y, 28, text_color);
    }
}

void GameUnload(Game *game) {
    for (int i = 0; i < game->snake_count; i++) {
        SnakeUnload(&game->snakes[i]);
    }
    UnloadSound(game->eat_sound);
    UnloadSound(game->game_over_sound);
    CloseAudioDevice();
}
