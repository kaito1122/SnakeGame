#include <ncurses.h>

//Structure to hold properties of obstacle
struct Obstacle {
    int x;
    int y;
    int size;
    struct Obstacle* next;
};

typedef struct Obstacle Obstacle;

//Function prototypes
void add_new_obs(Obstacle* obs, Obstacle* new_obs);
bool obs_exists(Obstacle* foods, int x, int y);
Obstacle* create_obstacle(int x, int y, int size);
void draw_obs(Obstacle* obs, int x_offset, int y_offset, int width, int height);