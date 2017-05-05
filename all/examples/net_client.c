#include <stdio.h>
#include "dconnect.h"




int main() {
	// Создаём пакет, способный хранить строку длиной 128 char
	UPACK_HEAD* temp_pack = make_UPACK(128);
	// Создаём адрес
	HR_ADDRESS serv_addr;
	// Локальный штамп "времени" последнего отправленного пакета
	TICK_TYPE temp_stamp;

	// Вводим IP-адрес сервера
	printf("Server IP: ");
	scanf("%s", serv_addr.ip);
	// Порт выставляется общий
	serv_addr.port = NET_PORT;

	// Подключаем сеть (создаём сокет); 1 - код типа "Клиент"
	if (d_all_connect(1) < 0) {
		return -1;
	}
	// Подключаемся к серверу. Функция отправляет и принимает несколько пакетов
	if (d_client_connect(serv_addr) < 0) {
		printf("Error dconnect!\n");
		return -1;
	}

	printf("\n");
	while (1) {
		// Создаём пакет для отправки
		temp_pack ->type = DP_INFO;
		printf("Stamp: ");
		scanf("%llu", (long long unsigned*)&(temp_pack ->stamp));
		temp_stamp = temp_pack ->stamp;
		printf("Data: ");
		scanf("%s", temp_pack ->data);

		// Отправляем пакет и печатаем данные об этом
		printf("Send success (0)? %d\n",
				d_client_send(temp_pack, sizeof(*temp_pack),
						NET_REPEAT_CLIENT));

		// Пока ответный пакет не получен, пробуем получить его.
		// У клиента эта функция получает пакет любого типа,
		// но притом проверяет, что пакет пришёл от сервера.
		// Если 'tick' > 0, то все не GAME пакеты отбрасываются,
		// а у оставшихся проверяется, что их 'stamp' >= переданного 'tick'
		while (d_client_get(temp_pack, sizeof(*temp_pack), temp_stamp) == -2) {
			continue;
		}
		// Печатаем ответ сервера
		printf("Server response: %s\n", temp_pack ->data);
		printf("\n");
	}

	// Эти строки никогда не выполнятся, цикл бесконечный
	return 0;
}
