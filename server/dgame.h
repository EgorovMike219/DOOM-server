#ifndef DGAME_H
#define DGAME_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../all/gamekey.h"

#include "dgame_settings.h"
#include "dlog.h"




// ========================================================================== //
// TYPES
// ========================================================================== //

/**
 * @brief Weapon entity
 *
 * @note 'type' is an entity code defined in `gamekey.h`
 */
typedef struct Weapon_Entity_t {
	char type;
	char name[LENGTH_NAME];
	int damage;
	int range;
	int charge;
	TICK_TYPE delay;
} WEAPON;


/**
 * @brief Unit entity
 *
 * @note 'type' is an entity code defined in `gamekey.h`
 */
typedef struct Unit_Entity_t {
	int id;
	char type;
	char name[LENGTH_NAME];
	
	int x;
	int y;
	int health;
	WEAPON weapon;
	
	TICK_TYPE next_action_tick;
	TICK_TYPE move_delay;
} UNIT;


/**
 * @brief Level cell
 */
typedef struct Level_t {
	char representation;
	WEAPON weapon;
	UNIT* units[UNITS_PER_CELL_MAX];
} CELL;




// ========================================================================== //
// PARAMETERS AND CONTAINERS
// ========================================================================== //

/// In-game time
TICK_TYPE tick;


/// Level storage
CELL* level;

int level_width;

int level_height;

int level_active_health_reduction;

int level_passive_health_reduction;

/// Number of turns any unit may stay alive performing no actions
int level_passive_turns;


/// Unit storage
UNIT* units;

/// Unit commands storage
char* units_cmd;

int units_total;


/// Default configuration of bomb
WEAPON bomb_default;

/// Default configuration of healing pack
WEAPON heal_default;

/// Default configuration of poisoning pack
WEAPON poison_default;




// ========================================================================== //
// GAME PROCESSING
// ========================================================================== //

/**
 * @return Position of a given cell in 'level' array
 *
 * @param x
 * @param y
 */
int d_level_pos(int x, int y);




/**
 * @brief Load game level from file
 *
 * @param pathname
 *
 * @return 0 if successful
 * @return -1 for errors
 *
 * @note Logging inside
 */
int d_level_load(char* pathname);




/**
 * @brief Use weapon in a given cell
 * @note 'weapon.charge' must be >= 1 and will be decreased by 1
 * @note 'weapon.type' == ENTITY_EMPTY is allowed (returns 1)
 *
 * @param weapon
 * @param x
 * @param y
 *
 * @return 0 if weapon is used successfully
 * @return 1 if weapon is out of charge or 'WEAPON.type' is ENTITY_EMPTY
 * @return -1 for errors
 */
int d_weapon_activate(WEAPON *weapon, int x, int y);




/**
 * @brief Move unit to a given position and process all effects (weapons, etc.)
 * @note No health debuffs (level_*_health_reduction) are applied
 * @note Weapons may be activated by this procedure
 * @note Dead units will NOT be removed from the 'level' storage
 *
 * @param unit
 * @param x Destination x
 * @param y Destination y
 *
 * @return 0 if successful and unit stayed alive
 * @return 1 if successful and unit died or if unit is already dead
 * @return 2 if unit is unable to move at the moment
 * @return 3 if movement to given position is impossible
 * @return -1 for errors
 */
int d_unit_move(UNIT* unit, int x, int y);


/**
 * @brief Use weapon a given unit has
 * @note Dead units MAY use weapons if this function is called
 *
 * @param unit
 *
 * @return 0 if successful and given unit stayed alive
 * @return 1 if successful and given unit died
 * @return 2 if weapon cannot be used at the moment by a given unit
 * @return 3 if weapon has not enough charges
 * @return -1 for errors
 */
int d_unit_use_weapon(UNIT* unit);


/**
 * @brief Find a random position to place a unit to
 * @note 'level' must be initialized before the execution
 *
 * @param x
 * @param y
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_unit_get_position(int* x, int* y);




/**
 * @brief Process given command for a given unit
 * @note Command may or may not be executed, according to the rules inside
 *
 * @param cmd Command: one of pre-defined in `gamekey.h` constants 'CMD_*'
 * @param unit
 *
 * @return 0 if given unit stayed alive
 * @return 1 if given unit died or had already been dead
 * @return -1 for errors
 */
int d_unit_process_command(char cmd, UNIT* unit);


/**
 * @brief Update game situation and write logs
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_game_update(void);


/**
 * @brief Test if the game is over and increase 'tick' by 1
 * @note This function does NOT update the game by calling 'd_game_update()'
 *
 * @return 0 if game should continue
 * @return 1 if game is over and some player is a winner
 * @return -1 if game is over and there is no winner
 * @return -2 for errors
 */
int d_game_refresh(void);


/**
 * @brief Safely end the game processing
 * @note Safe to call at any moment
 */
void d_game_shutdown(void);




#endif //DGAME_H
