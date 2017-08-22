#ifndef GAMEKEY_H
#define GAMEKEY_H

// ========================================================================== //
// SETTINGS
// ========================================================================== //

#include <stdint.h>  // Required for TICK_TYPE


/// Maximum length of all names in the game (including /0)
#define LENGTH_NAME 5

/// Maximum length of a command send via network (including \0)
#define LENGTH_NET_COMMAND (8 + 1)

/// Maximum length of game information send via network (including \0)
#define LENGTH_NET_INFO (4 + 1 + 2 + 1 + (LENGTH_NAME - 1) + 1 + 4 + 1 + LENGTH_NET_COMMAND)


#define CLIENT_FIELD_HEIGHT 11

#define CLIENT_FIELD_WIDTH 11

#define CLIENT_FIELD_AREA (CLIENT_FIELD_HEIGHT * CLIENT_FIELD_WIDTH)


/// Tick length (in seconds)
#define TICK_LENGTH 0.300f


// ========================================================================== //
// CONSTANTS
// ========================================================================== //

// Entity codes //
#define ENTITY_EMPTY ' '
#define ENTITY_WALL '#'
#define ENTITY_PLAYER 'p'
#define ENTITY_ENEMY 'e'

#define ENTITY_HEART '+'
#define ENTITY_HEART_NAME "HEAL"

#define ENTITY_POISON '-'
#define ENTITY_POISON_NAME "DRUG"

#define ENTITY_BOMB '*'
#define ENTITY_BOMB_NAME "BOMB"


// Player commands //
#define CMD_W 'w'
#define CMD_A 'a'
#define CMD_S 's'
#define CMD_D 'd'
#define CMD_WEAPON 'b'
#define CMD_QUIT 'q'
#define CMD_NONE ' '




// ========================================================================== //
// TYPES
// ========================================================================== //

/// Tick, an in-game time equivalent
#define TICK_TYPE uint64_t




#endif //GAMEKEY_H
