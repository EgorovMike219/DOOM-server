#ifndef DCLIENT_SETTINGS_H
#define DCLIENT_SETTINGS_H




// Game field parameters
#define CLIENT_FIELD_HEIGHT 10
#define CLIENT_FIELD_WIDTH 10
#define CLIENT_FIELD_AREA (CLIENT_FIELD_WIDTH * CLIENT_FIELD_HEIGHT)

// Additional graphics parameters
#define CLIENT_FIELD_BORDER 1
#define CLIENT_INFO_SIZE 3

#define DISPLAY_BACKGROUND 1
#define DISPLAY_ID_BORDER 2
#define DISPLAY_ID_FIELD 3
#define DISPLAY_ID_INFO 4
#define DISPLAY_ID_HEALTH 5

// Various parameters
#define CLIENT_REDRAW_TICK_MODULE 50
#define CLIENT_KEYTRYES 16
#define CLIENT_ERROR_WAIT 5.000




// TODO: Remove from this file
#define CMD_W 'w'
#define CMD_A 'a'
#define CMD_S 's'
#define CMD_D 'd'
#define CMD_BOMB 'b'
#define CMD_QUIT 'q'
#define CMD_NONE ' '




#endif //DCLIENT_SETTINGS_H
