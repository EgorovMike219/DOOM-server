#include "game.h"
#include "dconnect.h"
#include "ipToId.h"
#include "level.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>

void CommandsIn(void* arg) {
    HR_ADDRESS temp_addr;
    UPACK_HEAD* temp_pack = make_UPACK(1);

    while (isGameActive) {

        d_server_get(0, temp_pack, UPACK_SIZE(1),
                     &temp_addr);

        int id;
        resolve_hr(temp_addr, &id);

        if (*(temp_pack->data) == 'q') {
            remove_id(id);
        } else {
            unitsData[id].last_command = *(temp_pack->data);
        }

        d_all_delay(NET_PING);
    }

    pthread_exit(0);
}

void CommandsOut(void* arg) {
    UPACK_HEAD* temp_pack = make_UPACK(rowsCount * columnsCount);

    while (isGameActive) {
        for (int id = 0; id < maxUnitsCount; id++) {
            HR_ADDRESS addr;
            resolve_id(&addr, id);

            memcpy(temp_pack->data, levelData,
                   sizeof(char) * rowsCount * columnsCount);
            temp_pack->type = DP_GAME;
            d_server_send(temp_pack, UPACK_SIZE(rowsCount * columnsCount),
                          addr, NET_REPEAT_SERVER);
        }

        for (int id = 0; id < maxUnitsCount; id++) {
            HR_ADDRESS addr;
            resolve_id(&addr, id);

            char str_health[3];
            sprintf(str_health, "hp = %d", unitsData[id].health);

            memcpy(temp_pack->data, str_health,
                          sizeof(char) * 8);
            temp_pack->type = DP_GAME_INFO;
            d_server_send(temp_pack, UPACK_SIZE(rowsCount * columnsCount),
                          addr, NET_REPEAT_SERVER);
        }


        d_all_delay(NET_PING);
    }

    pthread_exit(0);
}

void CurrentGame(void* arg) {
    const size_t MAX_STRLEN = 128;
    // Создаём пакет, способный хранить строку длиной 128 char
    UPACK_HEAD* temp_pack = make_UPACK(MAX_STRLEN);
    // Создаём адрес
    HR_ADDRESS temp_addr;

    pthread_t commands_in_thread, commands_out_thread;
    int status;
    int curr_users_num = 0;

    while (true) {
        // Пока пакет не получен, пробуем его получить (функция неблокирующая)
        // У сервера эта функция выполняет ровно одно получение пакета
        // Если указан другой 'mode', функция уничтожит все неподходящие пакеты
        if (d_server_get(0, temp_pack, UPACK_SIZE(MAX_STRLEN),
                         &temp_addr) == -2) {
            continue;
        }

        // Если кто-то хочет подключиться (что следует из типа пакета)
        if (temp_pack->type == DP_CONNECT) {
            // Проверим, что он действительно хочет подключиться
            if (temp_pack->stamp == DP_S_ASK) {
                if (curr_users_num != maxUnitsCount + 1) {
                    if (!check_hr(temp_addr)) {
                        add_hr(temp_addr, curr_users_num);
                        curr_users_num += 1;
                    }

                    // Заменим 'stamp' согласно протоколу и отправим пакет
                    temp_pack->stamp = DP_S_SUCCESS;
                    d_server_send(temp_pack, UPACK_SIZE(MAX_STRLEN),
                                  temp_addr, NET_REPEAT_SERVER);
                } else {
                    // сессия переполнена
                    temp_pack->stamp = DP_S_ERROR;
                    d_server_send(temp_pack, UPACK_SIZE(MAX_STRLEN),
                                  temp_addr, NET_REPEAT_SERVER);
                }
            }
        }
        else {
            // pass
            // поток не хочет подключаться
        }

        if (curr_users_num == maxUnitsCount) {
            Initialize("./map.txt");

            status = pthread_create(&commands_in_thread, NULL, CommandsIn, NULL);
            if (status != 0) {
                fprintf(stderr, "can't create commands thread");
                exit(-1);
            }

            status = pthread_create(&commands_out_thread, NULL, CommandsOut, NULL);
            if (status != 0) {
                fprintf(stderr, "can't create stats thread");
                exit(-1);
            }

            for (int id = 0; id < maxUnitsCount; id++) {
                HR_ADDRESS addr;
                resolve_id(&addr, id);

                temp_pack->type = DP_GAME_PREPARE;
                d_server_send(temp_pack, UPACK_SIZE(1),
                              addr, NET_REPEAT_SERVER);
            }

            d_all_delay(5.0);

            for (int id = 0; id < maxUnitsCount; id++) {
                HR_ADDRESS addr;
                resolve_id(&addr, id);

                char temp = 0;
                temp_pack->type = DP_GAME_BEGIN;
                memcpy(temp_pack->data, &temp, 1);
                d_server_send(temp_pack, UPACK_SIZE(sizeof(int) * 2),
                              addr, NET_REPEAT_SERVER);
            }

            pthread_join(commands_in_thread, NULL);
            pthread_join(commands_out_thread, NULL);

            curr_users_num = 0;
        }
    }

    free(temp_pack);
}


void Daemon() {
    int status;
    pthread_t game_thread;

    //текущая сессия сервера
    status = pthread_create(&game_thread, NULL, CurrentGame, NULL);
    if (status != 0) {
        fprintf(stderr, "can't create game thread");
        exit(-1);
    }
}

int main() {
    Daemon();
    return 0;
}