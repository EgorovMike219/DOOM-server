#include <stdio.h>
#include "dconnect.h"








int main() {
	// Создаём пакет, способный хранить строку длиной 128 char
	UPACK_HEAD* temp_pack = make_UPACK(128);
	// Создаём адрес
	HR_ADDRESS temp_addr;

	// Подключаем сеть (создаём сокет); 0 - код типа "Сервер"
	if (d_all_connect(0) < 0) {
		return -1;
	}

	while (1) {
		// Пока пакет не получен, пробуем его получить (функция неблокирующая)
		// У сервера эта функция выполняет ровно одно получение пакета
		// Если указан другой 'mode', функция уничтожит все неподходящие пакеты
		if (d_server_get(0, temp_pack, sizeof(*temp_pack), &temp_addr) == -2) {
			continue;
		}
		// Печатаем отправителя в обычном IPv4 виде (IP:порт)
		printf("Adresser: %s:%d\n", temp_addr.ip, temp_addr.port);

		// Если кто-то хочет подключиться (что следует из типа пакета)
		if (temp_pack ->type == DP_CONNECT) {
			// Проверим, что он действительно хочет подключиться
			if (temp_pack ->stamp == DP_S_ASK) {
				// Заменим 'stamp' согласно протоколу и отправим пакет
				temp_pack ->stamp = DP_S_SUCCESS;
				printf("Connect reponse success (0)? %d\n", d_server_send(temp_pack,
						sizeof(*temp_pack), temp_addr, NET_REPEAT_ONE));
			}
		}
		else {
			// Печатаем пакет в удобном виде на stdout
			printf("Type and Stamp: %ld | %lld\n",
					(long)temp_pack ->type, (long long)temp_pack ->stamp);
			printf("Strlen and Data: %u | %s\n",
					(unsigned)strlen(temp_pack ->data), temp_pack ->data);

			// Отправляем ответ в виде GAME-пакета и печатаем результат отправки
			temp_pack ->type = DP_GAME;
			printf("Data repsonse success (0)? %d\n",
					d_server_send(temp_pack, sizeof(*temp_pack), temp_addr,
							NET_REPEAT_SERVER));
		}
		printf("\n");
	}

	// Эти строки никогда не выполнятся, цикл бесконечный
	free(temp_pack);
	return 0;
}



