#include "game.h"
#include "raylib.h"
#include <math.h>

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

static void GameUpdateSpeed(Game *game) {
    int level = game->score / 50;
    float interval = game->base_interval - (float)level * 0.003f;
    if (interval < game->min_interval) {
        interval = game->min_interval;
    }
    game->move_interval = interval;
}

static Rectangle GameGetPlayButton(const Game *game) {
    int width = 220;
    int height = 54;
    int x = (game->screen_width - width) / 2;
    int y = (game->screen_height - height) / 2 + 30;
    return (Rectangle){ (float)x, (float)y, (float)width, (float)height };
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

static void GameReset(Game *game) {
    SnakeInit(&game->snake, game->grid_size, game->screen_width, game->screen_height);
    FoodSpawn(&game->food, &game->snake, game->grid_size, game->screen_width, game->screen_height);
    game->score = 0;
    game->is_game_over = 0;
    game->is_paused = 0;
    game->move_timer = 0.0f;
    game->move_interval = game->base_interval;
}

void GameInit(Game *game, int screen_width, int screen_height) {
    game->screen_width = screen_width;
    game->screen_height = screen_height;
    game->grid_size = 20;
    game->base_interval = 0.30f;
    game->min_interval = 0.20f;
    game->is_intro = 1;

    InitAudioDevice();
    game->eat_sound = CreateBeepSound(640.0f, 0.08f);
    game->game_over_sound = CreateBeepSound(180.0f, 0.18f);

    GameReset(game);
}

void GameUpdate(Game *game) {
    if (game->is_intro) {
        Rectangle button = GameGetPlayButton(game);
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, button)) {
            game->is_intro = 0;
            GameReset(game);
        } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            game->is_intro = 0;
            GameReset(game);
        }
        return;
    }

    if (game->is_game_over) {
        if (IsKeyPressed(KEY_ENTER)) {
            GameReset(game);
        }
        return;
    }

    if (IsKeyPressed(KEY_P)) {
        game->is_paused = !game->is_paused;
    }

    if (game->is_paused) {
        return;
    }

    // Update direction from input.
    if (IsKeyPressed(KEY_UP)) {
        SnakeSetDirection(&game->snake, 0, -1);
    } else if (IsKeyPressed(KEY_DOWN)) {
        SnakeSetDirection(&game->snake, 0, 1);
    } else if (IsKeyPressed(KEY_LEFT)) {
        SnakeSetDirection(&game->snake, -1, 0);
    } else if (IsKeyPressed(KEY_RIGHT)) {
        SnakeSetDirection(&game->snake, 1, 0);
    }

    game->move_timer += GetFrameTime();
    if (game->move_timer >= game->move_interval) {
        game->move_timer = 0.0f;

        SnakeMove(&game->snake);

        if (SnakeCheckWallCollision(&game->snake, game->grid_size, game->screen_width, game->screen_height) ||
            SnakeCheckSelfCollision(&game->snake)) {
            game->is_game_over = 1;
            PlaySound(game->game_over_sound);
            return;
        }

        if (SnakeCheckFoodCollision(&game->snake, &game->food)) {
            SnakeGrow(&game->snake);
            game->score += 10;
            GameUpdateSpeed(game);
            PlaySound(game->eat_sound);
            FoodSpawn(&game->food, &game->snake, game->grid_size, game->screen_width, game->screen_height);
        }
    }
}

void GameDraw(const Game *game) {
    const Color text_color = (Color){230, 230, 230, 255};

    GameDrawGrid(game);
    if (!game->is_intro) {
        FoodDraw(&game->food, game->grid_size);
        SnakeDraw(&game->snake, game->grid_size);

        DrawText(TextFormat("Score: %d", game->score), 12, 12, 20, text_color);
        DrawText(TextFormat("Level: %d", (game->score / 50) + 1), 12, 36, 18, text_color);
    }

    if (game->is_intro) {
        const char *title = "Snake";
        int title_width = MeasureText(title, 48);
        DrawText(title, (game->screen_width - title_width) / 2, 120, 48, text_color);

        Rectangle button = GameGetPlayButton(game);
        Color button_color = (Color){60, 120, 80, 255};
        DrawRectangleRec(button, button_color);
        DrawRectangleLinesEx(button, 2.0f, (Color){100, 170, 120, 255});

        const char *label = "Play";
        int label_width = MeasureText(label, 28);
        int label_x = (int)(button.x + (button.width - label_width) / 2);
        int label_y = (int)(button.y + (button.height - 28) / 2);
        DrawText(label, label_x, label_y, 28, text_color);

        const char *hint = "Click Play or press Enter";
        int hint_width = MeasureText(hint, 18);
        DrawText(hint, (game->screen_width - hint_width) / 2, (int)(button.y + button.height + 20), 18, text_color);
        return;
    }

    if (game->is_game_over) {
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
    SnakeUnload(&game->snake);
    UnloadSound(game->eat_sound);
    UnloadSound(game->game_over_sound);
    CloseAudioDevice();
}
