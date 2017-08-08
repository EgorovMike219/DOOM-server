#ifndef DGAME_H
#define DGAME_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../all/gamekey.h"

#include "dgame_settings.h"
#include "dstatistics.h"




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
	char name[NAME_LENGTH];
	int damage;
	int range;
	int charge;
} WEAPON;


/**
 * @brief Unit entity
 *
 * @note 'type' is an entity code defined in `gamekey.h`
 */
typedef struct Unit_Entity_t {
	int id;
	char type;
	char name[NAME_LENGTH];
	
	int x;
	int y;
	int health;
	WEAPON weapon;
	
	TICK_TYPE last_action_stamp;
	TICK_TYPE action_delay;
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
CELL** level;

int level_width;

int level_height;

int level_active_health_reduction;

int level_passive_health_reduction;


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
 * @brief Load game level from file
 *
 * @param pathname
 */
void d_level_load(char* pathname);




/**
 * @brief Use weapon ON (not by!) selected unit
 * @note 'WEAPON.charge' must be >= 1 and will be decreased by 1
 *
 * @param weapon
 * @param unit
 *
 * @return 0 if weapon is used successfully
 * @return 1 if weapon is out of charge
 * @return -1 for errors
 */
int d_weapon_operate(WEAPON* weapon, UNIT* unit);




/**
 * @brief Move unit to a given position and process all effects (weapons, etc.)
 *
 * @param unit
 * @param x Destination x
 * @param y Destination y
 *
 * @return 0 if successful and unit stayed alive
 * @return 1 if successful and unit died
 * @return 2 if unit is unable to move at the moment
 * @return -1 for errors or if movement is impossible
 */
int d_unit_move(UNIT* unit, int x, int y);


/**
 * @brief Use weapon a given unit has
 *
 * @param unit
 *
 * @return 0 if successful and given unit stayed alive
 * @return 1 if successful and given unit died
 * @return 2 if weapon cannot be used at the moment
 * @return 3 if weapon has not enough charges
 * @return -1 for errors
 */
int d_unit_use_weapon(UNIT* unit);




/**
 * @brief Process given command for a given unit
 *
 * @param cmd
 * @param unit
 *
 * @return 0 if successful and given unit stayed alive
 * @return 1 if given unit died (cmd may be successful or not)
 * @return -1 for errors
 */
int d_unit_process_command(char* cmd, UNIT* unit);


/**
 * @brief Update game situation
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int d_game_update();


/**
 * @brief Test if the game is over and increase 'tick' by 1 if it is not
 * @note This function does NOT update the game by calling 'd_game_update()'
 *
 * @return 0 if game should continue
 * @return -1 if game is over and there is no winner
 * @return 1 if game is over and some player is a winner
 */
int d_game_refresh();


/**
 * @brief Safely end the game processing
 */
void d_game_shutdown();




#endif //DGAME_H
