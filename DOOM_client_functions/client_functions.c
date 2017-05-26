#include <pthread.h>
#include "client_functions.h"
#include "dconnect.h"
#include "dconnect_settings.h"


void ClientStartGame() {
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

  endwin();
  return;
}

void* SendData() {
  while (true) {
    keyboard_key command;
    pack_to_send->stamp = tick;

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
      pack_to_send->data[0] = -1;
    }

    d_client_send(pack_to_send, UPACK_SIZE(sizeof(keyboard_key)), NET_REPEAT_CLIENT);

    if (command == quit) {
      endwin();
      break;
    }
    refresh();
  }
  return NULL;
}


void* GetData_and_RenderScreen() {

  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  attron(COLOR_PAIR(1));
  while (true) {
    //get data
    d_client_get(pack_to_receive, screenRows * screenColumns * sizeof(ConsoleSymbolData),
                tick);
    if (pack_to_receive->type == DP_CLIENT_STOP) {
      printw("The game is over");
      return NULL;
    }
    if (pack_to_receive->type != DP_GAME) {
      continue;
    }

    tick = pack_to_receive->stamp;
    //render it
    int row;
    int column;
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    memcpy(screenBuffer, pack_to_receive->data, screenRows *
           screenColumns * sizeof(ConsoleSymbolData));
    for (row = 0; row < screenRows; ++row) {
      for (column = 0; column < screenColumns; ++column) {
        printw("%s", screenBuffer[row][column].symbol);
      }
    }

    refresh();
  }
  attroff(COLOR_PAIR(1));
  return NULL;
}

