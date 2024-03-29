// Kaito Minami
// CS3520: Programming in C++

/*game.cpp -------
*
* Filename: game.cpp
* Description:
* Author: Adeel Bhutta
* Maintainer:
* Created: Sat Sep 12 13:16:12 2022
* Last-Updated: September 12 16:51 2022
*
*/

/* Commentary:
*
*
*
*/

/* Change log:
*
*
*/

/*Copyright (c) 2022 Adeel Bhutta
*
* All rights reserved.
*
* Additional copyrights may follow
*/
#include <ncurses.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "snake.hpp"
#include "food.hpp"
#include "game_window.hpp"
#include "key.hpp"
#include "game.hpp"
#include "obstacle.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

void generate_points(int *food_x, int *food_y, int width, int height, int x_offset, int y_offset, int size){
    *food_x = rand() % (width - x_offset - size) + x_offset + size;
    *food_y = rand() % (height - y_offset - size) + y_offset + size;
}

char welcomeScreen() {
    printf("\n\nWelcome to the snake game!\n");
    printf("The game control is:\n");
    printf("KEY_LEFT to go left\n");
    printf("KEY_RIGHT to go right\n");
    printf("KEY_UP to go up\n");
    printf("KEY_DOWN to go down\n");
    printf("s to start the game\n");
    printf("\nScoring guideline:\n");
    printf("If you eat a food O, score increases by 20 and length increase\n");
    printf("If you eat a food X, score decreases by 10 and length decrease\n");
    printf("If you eat a food +, no points but length increase by 2\n");
    printf("If you eat a food -, no points and length decrease by 2\n");
    printf("The score cannot go lower than 0\n");
    printf("And every time the score crosses 100 points, increase the speed 1.5 times\n");
    printf("Avoid the obstacle, noted by !\n");
    printf("\nDefault is Easy, but you can choose Difficulty\n");
    printf("Type a for Advanced, d for Difficult, and e for Easy before starting game\n");
    printf("\nGet bigger without crushing! Enjoy your game!\n");

    char input = std::cin.get();
    char difficulty = 'e';
    while(tolower(input) != 's') {
        if(tolower(input) == 'a' || tolower(input) == 'd' || tolower(input) == 'e') {
            difficulty = input;
        }
        input = std::cin.get();
    }
    return difficulty;
}

void save_best_10(int score) {
    std::string line;
    int num, i=0;
    // I used STL multiset to sort and maintain descending top10
    std::multiset<int, std::greater<int>> s1;

    // load the previous scores
    std::ifstream top10;
    top10.open("saves/save_best_10.game");
    while(getline(top10, line)) {
        std::istringstream ss(line);
        while(ss>>num) {
            s1.insert(num);
        }
    }

    // insert the new score and erase the overall lowest score
    s1.insert(score);
    s1.erase(--s1.end());

    // save the new top 10 and show leaderboard
    std::ofstream new_top10;
    new_top10.open("saves/save_best_10.game");
    for(int score:s1){
        new_top10<<score<<"\n";
        mvprintw(10+i, 60,"%i", score);
        i++;
    }
}

void game(){
    enum State state = INIT; // Set the initial state
    static int x_max, y_max; // Max screen size variables
    static int x_offset, y_offset; // distance between the top left corner of your screen and the start of the board
    gamewindow_t *window; // Name of the board
    Snake *snake; // The snake
    Food *foods,*new_food; // List of foods (Not an array)
    Obstacle *obs, *new_obs;

    const int height = 30; 
    const int width = 70;
    char ch;

    struct timespec timeret;
    timeret.tv_sec = 0;
    //timeret.tv_nsec = 999999999/4; // I switched this to INIT for a convenience of game speed altering
    double nsec = 999999999/4;

    char diff_input = welcomeScreen();

    int score = 0, speed_point = 0, length=3, lives=3;
    int x_prev, y_prev;
    bool delay = false;

    while(state != EXIT){
        switch(state){
            case INIT: {
                initscr();
                start_color();
                nodelay(stdscr, TRUE); //Dont wait for char
                noecho(); // Don't echo input chars
                getmaxyx(stdscr, y_max, x_max);
                keypad(stdscr, TRUE); // making keys work
                curs_set(0); // hide cursor
                timeout(100);

                // initial snake length
                length = 3;

                // Setting height and width of the board
                x_offset = (x_max / 2) - (width / 2);
                y_offset = (y_max / 2) - (height / 2);

                //Init board
                window = init_GameWindow(x_offset, y_offset, width, height);
                draw_Gamewindow(window);

                // Init snake
                snake = init_snake(x_offset + (width / 2), y_offset + (height / 2));

                // difficulty altering: snake speed, obstacle amount, food amount
                // It is gradual change as randomized, but since minimum radius is 3, meaning 5x5. I thought it is fair
                // to increase obstacles one by one
                int quant_init, food_amount;
                if(diff_input == 'e') {
                    snake->speed = 1;
                    quant_init = 3; // default 3-10
                    food_amount = 10;
                } else if(diff_input == 'd') {
                    // add food, obstacle ++
                    snake->speed = 2;
                    quant_init = 4;
                    food_amount = 15;
                } else if(diff_input == 'a') {
                    // add food, obstacle --
                    snake->speed = 5;
                    quant_init = 5;
                    food_amount = 20;
                }

                // set the game speed based on snake speed
                timeret.tv_nsec = nsec / snake->speed;

                // Generate Obstacles
                int obs_x, obs_y, obs_size;
                obs_size = rand()%3 + 3;
                generate_points(&obs_x, &obs_y, width, height, x_offset, y_offset, obs_size);
                obs = create_obstacle(obs_x, obs_y, obs_size);

                // Generate rest of obstacles to randomized amount
                int quant = rand()%(10-3) + quant_init;
                for(int i=0;i<quant; i++) {
                    obs_size = rand() % 3 + 3;
                    while(obs_exists(obs, obs_x-obs_size, obs_y)||obs_exists(obs, obs_x+obs_size, obs_y)||obs_exists(obs, obs_x, obs_y-obs_size)||obs_exists(obs, obs_x, obs_y+obs_size)) {
                        generate_points(&obs_x, &obs_y, width, height, x_offset, y_offset, obs_size);
                    }
                    new_obs = create_obstacle(obs_x, obs_y, obs_size);
                    add_new_obs(obs, new_obs);
                }

                // Init foods
                int food_x, food_y, i;
                enum Type type;

                //Generate foods
                generate_points(&food_x, &food_y, width, height, x_offset, y_offset, 1);
                Type food_option[4] = {Increase, Decrease, Longer, Shorter};
                int idx = rand()%4;
                type = food_option[idx]; // Randomly deciding type of food
                foods = create_food(food_x, food_y, type);
                for(i = 0; i < food_amount; i++){
                    generate_points(&food_x, &food_y, width, height, x_offset, y_offset, 1);
                    while (food_exists(foods,food_x, food_y) || obs_exists(obs, food_x, food_y))
                        generate_points(&food_x, &food_y, width, height, x_offset, y_offset, 1);
                    type = food_option[rand()%4];
                    new_food = create_food(food_x, food_y, type);
                    add_new_food(foods, new_food);
                }

                state = ALIVE;
                break;
            }
            case ALIVE: {
                // action
                ch = get_char();
                char dummy;
                if(tolower(ch) == 'q') {
                    // quit
                    state = DEAD;
                    break;
                } else if (tolower(ch) == 'p') {
                    // pause screen
                    std::cin>>dummy;
                    if(tolower(dummy) == 'q') {
                        // quit
                        state = DEAD;
                        break;
                    }
                }

                /* Write your code here */
                Snake* end = snake;
                while(end->next) {
                    end = end->next; // get the tail
                }
                x_prev = end->x;
                y_prev = end->y;

                // snake moves!
                snake = move_snake(snake, ch);

                end = snake;
                while(end->next) {
                    end = end->next; // get the tail
                }

                // Longer food is yummy that I grow one more after eating
                if(delay) {
                    delay = false;
                    end->next = create_tail(x_prev, y_prev);
                    length++;
                    if(food_exists(foods,snake->x,snake->y) && (food_type(foods,snake->x,snake->y) == Increase || food_type(foods,snake->x,snake->y) == Longer)) {
                        delay = true;
                    }
                }

                // snake eats food!
                int food_x, food_y;
                if(food_exists(foods, snake->x, snake->y)) {
                    length++;
                    switch(food_type(foods, snake->x, snake->y)) {
                        case Increase: {
                            if(!delay) {
                                score += 20;
                                end->next  = create_tail(x_prev, y_prev);
                            } else {
                                length--;
                            }
                            break;
                        }
                        case Decrease: {
                            score -= 10;
                            length -= 2;
                            remove_tail(snake);
                            break;
                        }
                        case Longer: {
                            if(!delay) {
                                end->next  = create_tail(x_prev, y_prev);
                                delay = true;
                            }
                            break;
                        }
                        case Shorter: {
                            length -= 3;
                            remove_tail(snake);
                            remove_tail(snake);
                            break;
                        }
                    } // food calc
                    foods = remove_eaten_food(foods, snake->x, snake->y);
                    while(food_exists(foods,food_x,food_y) || obs_exists(obs, food_x, food_y)) {
                        generate_points(&food_x, &food_y, width, height, x_offset, y_offset, 1);
                    }
                    Type food_option[4] = {Increase, Decrease, Longer, Shorter};
                    int idx = rand()%4;
                    Type type = food_option[idx]; // Randomly deciding type of food
                    new_food = create_food(food_x, food_y, type);
                    add_new_food(foods, new_food);
                }

                // death penalties
                // snake crushed on walls
                if((snake->x == x_offset || snake->x == width + x_offset) || (snake->y == y_offset || snake->y == height + y_offset)) {
                    lives--;
                    state = INIT;
                } else if (eat_itself(snake)) {
                     // snake collided with itself
                    lives--;
                    state = INIT;
                } else if (!snake->next) {
                    // head snake, no tails
                    lives--;
                    state = INIT;
                }
                else if (obs_exists(obs, snake->x, snake->y)) {
                    // crushed into obstacles
                    lives--;
                    state = INIT;
                }

                // no more lives. reincarnation failed
                if(lives==0) {
                    state = DEAD;
                }

                // speed calc
                if((score - speed_point) >= 100) {
                    snake->speed *= 1.5;
                    timeret.tv_nsec = nsec / snake->speed;
                    speed_point = score;
                }
                // score min set to 0. somehow unsigned did not work on this, so manually
                if(score<0) {
                    score = 0;
                }

                // Draw everything on the screen
                clear();
                mvprintw(20,20, "Key entered: %c", ch);
                draw_Gamewindow(window);
                draw_obs(obs, x_offset, y_offset, width, height);
                draw_snake(snake);
                draw_food(foods);
                mvprintw(0,0,"Score: %i, Length: %i, Lives: %i",score, length, lives);
                break;
            }
            case DEAD: {
                // game over message and final score
                mvprintw(10,30,"GAME OVER");
                mvprintw(11,30,"Your final score is: %i", score);

                // save the score and leaderboard for funsies
                mvprintw(9,60,"Leaderboard");
                save_best_10(score);

                // actual end game
                endwin();
                state = EXIT;
                break;
            }
        }
        refresh();
        nanosleep(&timeret, NULL);
    }
}
