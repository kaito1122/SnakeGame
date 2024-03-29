// Kaito Minami
// CS3520: Programming in C++

#include <cstdlib>
#include <ncurses.h>
#include <cstdbool>
#include "obstacle.hpp"

// obstacle x and y are the core. size is radius but for square.
Obstacle* create_obstacle(int x, int y, int size) {
    Obstacle* new_obstacle = (Obstacle*)malloc(sizeof(Obstacle));

    new_obstacle->x = x;
    new_obstacle->y = y;
    new_obstacle->size = size;
    new_obstacle->next = NULL;

    return new_obstacle;
}

//Check if obstacle exists at coordinates
bool obs_exists(Obstacle* obs, int x, int y){
    Obstacle* temp = obs;
    while(temp){
        for(int i = temp->y - temp->size - 1; i < temp->y + temp->size; i++) {
                for(int j = temp->x - temp->size - 1; j < temp->x + temp->size; j++) {
                    if (j == x && i == y) {
                        return true;
                    }
                }
        }
        temp = temp->next;
    }
    return false;
}

//Add new obstacle to end of obstacle list
void add_new_obs(Obstacle* obs, Obstacle* new_obs){
    Obstacle* temp = obs;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = new_obs;
}

// Display all the obstacle
void draw_obs (Obstacle* obs, int x_offset, int y_offset, int width, int height)
{   Obstacle* temp = obs;
    while(temp) {
        for(int i = temp->y - temp->size - 1; i < temp->y + temp->size; i++) {
            if (i > y_offset && i < y_offset + height)
                for(int j = temp->x - temp->size - 1; j < temp->x + temp->size; j++) {
                    if (j > x_offset && j < x_offset + width) {
                        init_pair(3, COLOR_RED, COLOR_BLACK);
                        attron(COLOR_PAIR(3));
                        mvprintw(i, j, "!");
                        attroff(COLOR_PAIR(3));
                    }
                }
        }
        temp = temp->next;
    }
}
