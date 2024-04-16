#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>
#include <random>

enum tile {
    EMPTY,
    STONE,
    START,
    GRASS,
    ENEMY,
    GOAL
};

enum goal {
    NIL,
    WIN,
    LOSE
};

struct map_data {
    std::vector<int> map;
    std::vector<int> enemies;
    int start;
};

const int MAP_WIDTH = 32;
const int MAP_HEIGHT = 32;
const int TILE_SIZE = 32;
const int DEBOUNCE = 10;
const int ENMY_DEBOUNCE = 100;
const int CURRENT_MONITOR = 0;
const int ENEMIES = 5;

#ifdef __WINDOWS__
    const int WIDTH = GetMonitorWidth(GetCurrentMonitor());
    const int HEIGHT = GetMonitorHeight(GetCurrentMonitor());
    const int WIDTH = GetMonitorWidth(GetCurrentMonitor());
    const int HEIGHT = GetMonitorHeight(GetCurrentMonitor());
#elif __LINUX__
    const int WIDTH = GetMonitorWidth(GetCurrentMonitor());
    const int HEIGHT = GetMonitorHeight(GetCurrentMonitor());
#else
    const int WIDTH = 1440;
    const int HEIGHT = 900;
#endif

map_data generateMap() {
    std::vector<int> map(MAP_WIDTH * MAP_HEIGHT, EMPTY);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, MAP_WIDTH * MAP_HEIGHT - 1);
    std::vector<int> enemies(ENEMIES, 0);
    int pos = 0;
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT / 2; i++) {
        map[dis(gen)] = STONE;
    }
    for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT / 2; i++) {
        map[dis(gen)] = GRASS;
    }
    for (int i = 0; i < ENEMIES; i++) {
        pos = dis(gen);
        map[pos] = ENEMY;
        enemies[i] = pos;
    }
    // Guarantee that the start and goal are generated
    int start = dis(gen);
    int goal = dis(gen);
    while (map[start] != EMPTY) {
        start = dis(gen);
    }
    while (map[goal] != EMPTY) {
        goal = dis(gen);
    }
    map[start] = START;
    map[goal] = GOAL;
    return {map, enemies, start};
}

void render_map(const std::vector<int>& map) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            switch (map[y * MAP_WIDTH + x]) {
                case EMPTY:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, Color {201, 164, 108, 255});
                    break;
                case GRASS:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGREEN);
                    break;
                case STONE:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
                    break;
                case START:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GREEN);
                    break;
                case ENEMY:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, YELLOW);
                    break;
                case GOAL:
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, RED);
                    break;
            }
        }
    }
}

int ai(const std::vector<int>& map, int player, int ai) {
    int player_x = player % MAP_WIDTH;
    int player_y = player / MAP_WIDTH;
    int ai_x = ai % MAP_WIDTH;
    int ai_y = ai / MAP_WIDTH;
    int dx = player_x - ai_x;
    int dy = player_y - ai_y;
    if (dx == 0 && dy == 0) {
        // turn position into index
        return player;
    }
    if (dx > 0 && map[ai_y * MAP_WIDTH + ai_x + 1] != STONE) {
        ai_x++;
    }
    if (dx < 0 && map[ai_y * MAP_WIDTH + ai_x - 1] != STONE) {
        ai_x--;
    }
    if (dy > 0 && map[(ai_y + 1) * MAP_WIDTH + ai_x] != STONE) {
        ai_y++;
    }
    if (dy < 0 && map[(ai_y - 1) * MAP_WIDTH + ai_x] != STONE) {
        ai_y--;
    }
    return ai_y * MAP_WIDTH + ai_x;
}

int main() {
    const float width = WIDTH;
    const float height = HEIGHT;
    const std::string title = "Hello, World!";
    map_data data = generateMap();
    int x, y, speed, score, ai_x, ai_y, debounce = 0;
    goal win = NIL;

    // create a camera that follows the player
    Camera2D camera = {};
    camera.target = {float(x) * TILE_SIZE + TILE_SIZE / 2, float(y) * TILE_SIZE + TILE_SIZE / 2};
    camera.offset = {width / 2, height / 2};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    x = data.start % MAP_WIDTH;
    y = data.start / MAP_WIDTH;
    speed = 4;

    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
        render_map(data.map);
        DrawCircle(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE / 2, BLUE);
        DrawCircle(ai_x, ai_y, TILE_SIZE / 2, ORANGE);
        debounce = int(GetTime() * GetFPS());
        if (debounce % ENMY_DEBOUNCE == 0 && win == NIL) {
            for (int i = 0; i < ENEMIES; i++) {
                data.start = y * MAP_WIDTH + x;
                data.enemies[i] = ai(data.map, data.start, data.enemies[i]);
                if (data.start == data.enemies[i]) {
                    win = LOSE;
                    std::cout << "Bot[" << i << "] got you!" << std::endl;
                    if (score > 0) {
                        score--;
                        continue;
                    }
                }
                ai_x = (data.enemies[i] % MAP_WIDTH) * TILE_SIZE + TILE_SIZE / 2;
                ai_y = (data.enemies[i] / MAP_WIDTH) * TILE_SIZE + TILE_SIZE / 2;
            }
        }
        switch (win){
            case NIL: {
                switch (debounce % DEBOUNCE % speed) {
                    case 0:
                        if (IsKeyDown(KEY_W) && y > 0 && data.map[(y - 1) * MAP_WIDTH + x] != STONE) {
                            y--;
                        }
                        if (IsKeyDown(KEY_S) && y < MAP_HEIGHT - 1 && data.map[(y + 1) * MAP_WIDTH + x] != STONE) {
                            y++;
                        }
                        if (IsKeyDown(KEY_A) && x > 0 && data.map[y * MAP_WIDTH + x - 1] != STONE) {
                            x--;
                        }
                        if (IsKeyDown(KEY_D) && x < MAP_WIDTH - 1 && data.map[y * MAP_WIDTH + x + 1] != STONE) {
                            x++;
                        }
                        break;
                    default:
                        break;
                }
                if (data.map[y * MAP_WIDTH + x] == GOAL) {
                    score++;
                    win = WIN;
                }
                camera.target = {float(x) * TILE_SIZE + TILE_SIZE / 2, float(y) * TILE_SIZE + TILE_SIZE / 2};
                break;
            }
            case WIN:  {
                int length = MeasureText("You Win!", 22);
                // middle of camera window
                DrawText("You Win!", camera.target.x - length / 2, camera.target.y, 22, BLACK);
                length = MeasureText("You Win!", 20);
                DrawText("You Win!", camera.target.x - length / 2, camera.target.y, 20, PURPLE);
                length = MeasureText("Press R to Restart", 20);
                DrawText("Press R to Restart", camera.target.x - length / 2, camera.target.y + 30, 20, PURPLE);
                break;
            }
            case LOSE:  {
                int length = MeasureText("You Lose!", 22);
                // middle of camera window
                DrawText("You Lose!", camera.target.x - length / 2, camera.target.y, 22, BLACK);
                length = MeasureText("You Lose!", 20);
                DrawText("You Lose!", camera.target.x - length / 2, camera.target.y, 20, PURPLE);
                length = MeasureText("Press R to Restart", 20);
                DrawText("Press R to Restart", camera.target.x - length / 2, camera.target.y + 30, 20, PURPLE);
                break;
            }
        }
        if (IsKeyPressed(KEY_R)) {
            data = generateMap();
            x = data.start % MAP_WIDTH;
            y = data.start / MAP_WIDTH;
            win = NIL;
        }
        std::string score_text = "Score: " + std::to_string(score);
        int length = MeasureText(score_text.c_str(), 20);
        DrawText(score_text.c_str(), camera.target.x - length / 2, camera.target.y - 30, 20, PURPLE);
        EndMode2D();
        EndDrawing();
    }
    return 0;
}