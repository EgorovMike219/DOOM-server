#ifndef DCLIENT_SETTINGS_H
#define DCLIENT_SETTINGS_H




/// Displayed border width/height
#define CLIENT_FIELD_BORDER 1

/// Displayed game info height
#define CLIENT_INFO_SIZE 4


/// Defined in main()
#define DISPLAY_BACKGROUND 1

/// Defined in main()
#define DISPLAY_ID_BORDER 2

/// Defined in main()
#define DISPLAY_ID_FIELD 3

/// Defined in main()
#define DISPLAY_ID_INFO 4

/// Defined in main()
#define DISPLAY_ID_HEALTH 5


/// A module at which some curses actions should be performed (during the game)
#define CLIENT_REDRAW_TICK_MODULE 1000

/// A number of wgetch() tries
#define CLIENT_KEYTRYES 16

/// Delay (in seconds) before exit at error (and in other cases, e.g. game over)
#define CLIENT_DELAY 5.000f




#endif //DCLIENT_SETTINGS_H
