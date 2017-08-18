cmake_minimum_required(VERSION 3.8)
project(DOOM-592)

set(CMAKE_C_STANDARD 99)



set(CURSES_NEED_NCURSES true)
find_package(Curses REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)



set(SERVER_SOURCE_FILES
	./all/dconnect.c
	./all/dconnect_settings.h
	./all/gamekey.h
	
	server/dgame.c
	server/dlog.c
	server/dserver.c
	server/dgame_settings.h
	)
add_executable(server ${SERVER_SOURCE_FILES})
target_link_libraries(server ${CURSES_LIBRARIES})
target_link_libraries(server pthread)


set(CLIENT_SOURCE_FILES
	./all/dconnect.c
	./all/dconnect_settings.h
	./all/gamekey.h

	./client/dclient.c
	./client/dclient_settings.h
	)
add_executable(client ${CLIENT_SOURCE_FILES})
target_link_libraries(client ${CURSES_LIBRARIES})
target_link_libraries(client pthread)


set(TSERVER_SOURCE_FILES
		./all/dconnect_settings.h
		./all/dconnect.c
		./all/gamekey.h
		
		./all/net_server.c
		)
add_executable(tserver ${TSERVER_SOURCE_FILES})

