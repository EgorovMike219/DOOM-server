#include <stdio.h>
#include "dconnect.h"








int main() {
	const size_t MAX_STRLEN = 128;
	// Создаём пакет, способный хранить строку длиной 128 char
	UPACK_HEAD* temp_pack = make_UPACK(MAX_STRLEN);
	// Создаём адрес
	HR_ADDRESS temp_addr;
	
	uint64_t tick = 1;

	// Подключаем сеть (создаём сокет); 0 - код типа "Сервер"
	if (d_all_connect(0) < 0) {
		return -1;
	}
	
	while (1) {
		while (d_server_get(0,
							temp_pack, UPACK_SIZE(MAX_STRLEN),
							&temp_addr) < 0) {}
		
		// Печатаем отправителя в обычном IPv4 виде (IP:порт)
		printf("From: %s:%d\n", temp_addr.ip, temp_addr.port);
		
		// Если кто-то хочет подключиться (что следует из типа пакета)
		if ((temp_pack->type == DP_CONNECT) ||
			(temp_pack->type == DP_RECONNECT)) {
			// Проверим, что он действительно хочет подключиться
			if (temp_pack->stamp == DP_S_ASK) {
				// Заменим 'stamp' согласно протоколу и отправим пакет
				temp_pack->stamp = DP_S_SUCCESS;
				printf("Connect response success (0)? %d\n\n",
					   d_server_send(temp_pack, UPACK_SIZE(1),
									 temp_addr, NET_REPEAT_SERVER));
				break;
			}
		}
	}
	
	while (d_server_get(0,
						temp_pack, UPACK_SIZE(MAX_STRLEN),
						&temp_addr) >= 0) {}
	
	d_all_delay(30.000);
	
	temp_pack ->type = DP_GAME_PREPARE;
	temp_pack ->stamp = 0;
	printf("Prepare sent(0)? %d\n\n",
		   d_server_send(temp_pack, UPACK_SIZE(1),
						 temp_addr, NET_REPEAT_SERVER));
	
	d_all_delay(15.000);
	
	temp_pack ->type = DP_GAME_BEGIN;
	temp_pack ->stamp = 0;
	printf("Begin sent(0)? %d\n\n",
		   d_server_send(temp_pack, UPACK_SIZE(1),
						 temp_addr, NET_REPEAT_SERVER));
	
	for (int i=0; i<100; i++) {
		temp_pack ->data[i] = (char)('0' + i % 10);
	}
	temp_pack ->data[100] = '\0';
	temp_pack ->type = DP_GAME;
	
	while (tick < 1000000) {
		temp_pack ->stamp = tick;
		printf("Send game data(0)? %d\n\n",
			   d_server_send(temp_pack, UPACK_SIZE(1),
							 temp_addr, NET_REPEAT_SERVER));
		tick += 1;
		d_all_delay(0.500);
	}
	
	

	// Эти строки никогда не выполнятся, цикл бесконечный
	free(temp_pack);
	return 0;
}



