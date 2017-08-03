cmake_minimum_required(VERSION 3.8)
project(DOOM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)



set(CURSES_NEED_NCURSES true)
find_package(Curses REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)



set(SERVER_SOURCE_FILES
	./all/dconnect.c
	./all/dconnect_settings.h

	./server/game.c
	./server/ipToId.c
	./server/level.c
	./server/unit.c
	./server/weapon.c
	./server/statistics.c
	./server/main.c
	)
add_executable(server ${SERVER_SOURCE_FILES})
target_link_libraries(server ${CURSES_LIBRARIES})
target_link_libraries(server pthread)


set(TSERVER_SOURCE_FILES
	./all/dconnect_settings.h
	./all/dconnect.c
	
	./all/net_server.c
	)
add_executable(tserver ${TSERVER_SOURCE_FILES})


set(CLIENT_SOURCE_FILES
	./all/dconnect.c
	./all/dconnect_settings.h

	./client/dclient.c
	./client/dclient_settings.h
	)
add_executable(client ${CLIENT_SOURCE_FILES})
target_link_libraries(client ${CURSES_LIBRARIES})
target_link_libraries(client pthread)
