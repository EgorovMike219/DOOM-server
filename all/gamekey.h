#ifndef GAMEKEY_H
#define GAMEKEY_H

// ========================================================================== //
// SETTINGS
// ========================================================================== //

#include <stdint.h>  // Required for TICK_TYPE


/// Maximum length of all names in the game
#define NAME_LENGTH 4

/// Maximum length of game information (send via network)
#define INFO_LENGTH 128


#define CLIENT_FIELD_HEIGHT 11

#define CLIENT_FIELD_WIDTH 11

#define CLIENT_FIELD_AREA CLIENT_FIELD_HEIGHT * CLIENT_FIELD_WIDTH




// ========================================================================== //
// CONSTANTS
// ========================================================================== //

// Entity codes //
#define ENTITY_EMPTY ' '
#define ENTITY_WALL '#'
#define ENTITY_PLAYER 'p'
#define ENTITY_ENEMY 'e'

#define ENTITY_HEART '+'
#define ENTITY_HEART_NAME ""

#define ENTITY_POISON '-'
#define ENTITY_POISON_NAME ""

#define ENTITY_BOMB '*'
#define ENTITY_BOMB_NAME ""


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
