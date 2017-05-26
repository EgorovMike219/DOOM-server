//
// Created by egorov on 30.04.17.
//
#include "game.h"
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "level.h"
#include <stdio.h>
#include <stdlib.h>

/*int main()
{
    initscr();                   // Переход в curses-режим
    clock_t old_time = clock();
    clock_t new_time;
    clock_t last_read_time = old_time;
    clock_t last_print_time = old_time;
    int print = 0;
    int framesCounter = 0;
    float framesTimeCounter = 0.0;
    int fps = 0;
    float deltaTime;
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    int c;
    noecho();
    while(1) {
        new_time = clock();
        deltaTime = (float)(new_time - old_time) / CLOCKS_PER_SEC;
        old_time = new_time;
        framesCounter++;
        framesTimeCounter += deltaTime;
        if (framesTimeCounter >= 1.0) {
            framesTimeCounter -= 1.0;
            fps = framesCounter;
            framesCounter = 0;
        }
        move(0,0);
        printw("%d",fps);  // Отображение приветствия в буфер
        if ((float)(new_time - last_read_time) / CLOCKS_PER_SEC > 0.001) {
            c = getch();  // не правильно считается время
            if (c != -1) {
                print = 1;
                last_print_time = new_time;
                move(1, 1);
                printw("%c", c);
            } else {
                if (print == 1) {
                    if ((float) (new_time - last_print_time) / CLOCKS_PER_SEC > 0.1) {
                        print = 0;
                        move(1, 1);
                        printw(" ");
                    }
                }
            }
            last_read_time = new_time;
        }
        refresh();                   // Вывод приветствия на настоящий экран
        if (c == 'q') {
            break;
        }
    }
    endwin();                    // Выход из curses-режима. Обязательная команда.
    return 0;
}*/

int main() {
    SetupSystem();
    Initialize("./map.txt");
    do {
        Render();
        Update();
    }
    while (isGameActive  == true);

    Shutdown();

    return 0;
}
