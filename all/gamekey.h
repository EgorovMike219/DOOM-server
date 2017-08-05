#ifndef GAMEKEY_H
#define GAMEKEY_H

// ========================================================================== //
// SETTINGS
// ========================================================================== //






// ========================================================================== //
// CONSTANTS
// ========================================================================== //

// Cell types //
#define CELL_EMPTY ' '
#define CELL_WALL '#'

#define CELL_PLAYER 'p'
#define CELL_ENEMY 'e'

#define CELL_HEART '+'
#define CELL_POISON '-'
#define CELL_BOMB '*'


// Player commands //
#define CMD_W 'w'
#define CMD_A 'a'
#define CMD_S 's'
#define CMD_D 'd'
#define CMD_BOMB 'b'
#define CMD_QUIT 'q'
#define CMD_NONE ' '




// ========================================================================== //
// TYPES
// ========================================================================== //

/// Tick, an in-game time equivalent
#define TICK_TYPE uint64_t




#endif //GAMEKEY_H
