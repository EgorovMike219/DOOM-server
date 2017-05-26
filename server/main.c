#include "game.h"
#include "dconnect.h"
#include <stdio.h>
#include <pthread.h>


void Daemon() {
    int status;
    p_thread game_thread;

    //текущая сессия сервера
    status = pthread_create(&game_thread, NULL, CurrentGame, NULL);
    if (status != 0) {
        fprintf(stderr, "can't create game thread");
        exit(ERROR_CREATE_THREAD);
    }
}


void CurrentGame() {
    const size_t MAX_STRLEN = 128;
    // Создаём пакет, способный хранить строку длиной 128 char
    UPACK_HEAD* temp_pack = make_UPACK(MAX_STRLEN);
    // Создаём адрес
    HR_ADDRESS temp_addr;

    p_thread commands_thread, stats_thraed;
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
                    if (CheckUniqueIp(temp_addr->ip)) {
                        AddUser(temp_addr->ip);
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

        if (curr_users_num == maxUnitsCounts) {
            Initialize("./map.txt");

            status = pthread_create(&commands_thread, NULL, ComandsProcessing, NULL);
            if (status != 0) {
                fprintf(sterr, "can't create commands thread");
                exit(ERROR_CREATE_THREAD);
            }

            status = pthread_create(&stats_thread, NULL, Stats, NULL);
            if (status != 0) {
                fprintf(sterr, "can't create stats thread");
                exit(ERROR_CREATE_THREAD);
            }

            for (i = 0; i < n; i++) {
                temp_pack->type = DP_GAME_PREPARE;
                d_server_send(temp_pack, UPACK_SIZE(1),
                              temp_addr, NET_REPEAT_SERVER);
            }

            d_all_delay(5.0);

            for (i = 0; i < n; i++) {
                temp_pack->type = DP_GAME_BEGIN;
                d_server_send(temp_pack, UPACK_SIZE(1),
                              temp_addr, NET_REPEAT_SERVER);
            }

            pthread_join(commands_thread);
            pthread_join(stats_thread);

            curr_user_num = 0;
        }
    }
}

void ComandsProcessing() {
    while (isGameActive) {

        d_all_delay(NET_PING);
    }
}

void StatsProcessing() {

}

CheckUniqueIP(char* ip_address) {

}

AddNewUser(char* ip_address) {

}