#include <pthread.h>
#include "dclient.h"
#include "dconnect.h"
#include "dconnect_settings.h"


void ClientStartGame() {
  tick = 1;
  game_is_on = true;
  HR_ADDRESS serv_addr;
  serv_addr.port = NET_PORT;
  printf("What ip-adress you want to connect?\n");
  scanf("%s", serv_addr.ip);
  if (d_all_connect(1) < 0) {
    printf("Server connection fatal error\n");
    exit(-1);
  }
  if (d_client_connect(serv_addr, 0) < 0) {
    printf("Server connection error\n");
    exit(-1);
  }
  pack_to_send = make_UPACK(sizeof(keyboard_key));
  pack_to_receive = make_UPACK(receive_pack_size);
  tick = 1;
  pack_to_send->type = DP_CLIENT_ACTION;


  initscr();
  start_color();
  nodelay(stdscr, true);
  keypad(stdscr, true);
  noecho();

  pthread_t send_thread;
  pthread_create(&send_thread, NULL, SendData, NULL);

  pthread_t receive_render_thread;
  pthread_create(&receive_render_thread, NULL, GetData_and_RenderScreen, NULL);


  pthread_join(send_thread, NULL);
  pthread_join(receive_render_thread, NULL);


  return;
}

void* SendData() {
  while (true) {
    keyboard_key command;
    pack_to_send->stamp = tick;
    halfdelay(1);

    //get keyboard command and send it to the server
    command = getch();
    switch (command) {
    case forward:
      pack_to_send->data[0] = forward;
      break;

    case left:
      pack_to_send->data[0] = left;
      break;

    case backward:
      pack_to_send->data[0] = backward;
      break;

    case right:
      pack_to_send->data[0] = right;
      break;

    case bomb:
      pack_to_send->data[0] = bomb;
      break;

    case quit:
      pack_to_send->data[0] = quit;
      break;

    default:
      pack_to_send->data[0] = '\0';
    }

    d_client_send(pack_to_send, UPACK_SIZE(sizeof(keyboard_key)), NET_REPEAT_CLIENT);

    if (command == quit) {
      game_is_on = false;
      break;
    }

  }
  return NULL;
}

void* GetData_and_RenderScreen() {

  while (true) {
    //get data
    d_client_get(pack_to_receive, UPACK_SIZE(receive_pack_size),
                tick);
    if (pack_to_receive->type == DP_CLIENT_STOP || !game_is_on) {
      endwin();
      system("clear");
      printf("THE GAME IS OVER\n");
      return NULL;
    }
    if (pack_to_receive->type != DP_GAME) {
      continue;
    }

    tick = pack_to_receive->stamp;
    //render it
    int row;
    int column;
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    char screenBuffer[10][10];
    memcpy(screenBuffer, pack_to_receive->data, receive_pack_size);
    move(0, 0);
    for (row = 0; row < screenRows; ++row) {
      for (column = 0; column < screenColumns; ++column) {
        printw("%c ", screenBuffer[row][column]);
      }
      printw("\n");
    }
    refresh();
  }
  attroff(COLOR_PAIR(1));
  return NULL;
}

