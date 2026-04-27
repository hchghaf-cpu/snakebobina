# Snake (Raylib, C99)

A simple Snake game built with Raylib.

## Build

From the workspace root:

```
gcc -std=c99 -Iinclude src/main.c src/game.c src/snake.c src/food.c -Llib -lraylib -lopengl32 -lgdi32 -lwinmm -o snake.exe
```

If your Raylib installation is in a different location, adjust `-I` and `-L`.

## Run

```
./snake.exe
```

## Controls

- Arrow keys: move
- Enter: start game or restart after Game Over
- Space: start game
- P: pause/resume

## Notes

- Sound effects are generated procedurally using Raylib's audio API.
```
