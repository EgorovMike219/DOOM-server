#include "game.h"
#include <ncurses.h>
/*void Game() {
    Initialize("./map.txt");
    //send message
    do {
        Update();
    } while (isGameActive  == true);

    Shutdown();
}*/
int main() {

    //принимаем пользователей
    //если какой-то пользователь начинаю игру
    //maxUnits = initialize
    //unitsData = initialize
    //thread_create(Game)
    //продолжаем принимать сигналы

    SetupSystem();
    unitsCount = 2;
    Initialize("./map.txt");
    do {
        Render();
        Update();
    }
    while (isGameActive  == true);

    Shutdown();


    return 0;
}
