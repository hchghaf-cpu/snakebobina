#ifndef FOOD_H
#define FOOD_H

struct Snake;

typedef struct Food {
    int x;
    int y;
} Food;

void FoodSpawn(Food *food, const struct Snake *snake, int grid_size, int screen_width, int screen_height);
void FoodDraw(const Food *food, int grid_size);

#endif
