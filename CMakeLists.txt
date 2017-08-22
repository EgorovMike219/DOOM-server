cmake_minimum_required(VERSION 3.8)
project(DOOM-592)

set(CMAKE_C_STANDARD 99)



set(CURSES_NEED_NCURSES true)
find_package(Curses REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)



set(SERVER_SOURCE_FILES
		all/gamekey.h
		all/dconnect_settings.h
		server/dgame_settings.h
		server/dserver_settings.h
		
		all/dconnect.c
		server/dgame.c
		server/dlog.c
		server/dserver.c
		)
add_executable(server ${SERVER_SOURCE_FILES})
target_link_libraries(server pthread)
target_link_libraries(server m)


set(CLIENT_SOURCE_FILES
		all/gamekey.h
		all/dconnect_settings.h
		client/dclient_settings.h
		
		all/dconnect.c
		client/dclient.c
		)
add_executable(client ${CLIENT_SOURCE_FILES})
target_link_libraries(client ${CURSES_LIBRARIES})
target_link_libraries(client pthread)
