#include "dgame.h"




// Storage definitions (for safe 'd_game_shutdown' call at any time)
CELL* level = NULL;
UNIT* units = NULL;
char* units_cmd = NULL;




/**
 * @brief Helper function for 'd_level_load'
 *
 * @param file
 * @param holder
 * @param setting
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int read_setting(FILE* file, int* holder, char* setting) {
	static char string[SETTING_LENGTH];
	static char dummy[1];
	static char value[10];
	if (fscanf(file, "%s%s%s", string, dummy, value) == EOF) {
		fprintf(stderr, "Invalid map: Not all settings provided\n");
		return -1;
	}
	if (strcmp(string, setting) != 0) {
		fprintf(stderr,
				"Invalid map: %s not found\n", setting);
		return -1;
	}
	*holder = (int)strtol(value, NULL, 10);
	return 0;
}




/**
 * @brief Fill cell using default settings for a given entity
 * @note This procedure does not place units (only their representation)
 *
 * @param cell
 * @param representation Cell entity code (as defined in gamekey.h).
 * Not all codes are allowed to be representation!
 *
 * @return 0 if successful
 * @return -1 for errors
 */
int fill_cell(CELL* cell, char entity) {
	if (cell == NULL) {
		return -1;
	}
	
	switch(entity) {
		case ENTITY_EMPTY:
		case ENTITY_WALL:
			cell ->weapon.type = ENTITY_EMPTY;
			cell ->representation = entity;
			break;
		case ENTITY_PLAYER:
		case ENTITY_ENEMY:
			cell ->weapon.type = ENTITY_EMPTY;
			cell ->representation = ENTITY_EMPTY;
			break;
		case ENTITY_HEART:
			cell ->weapon.type = ENTITY_HEART;
			strcpy(cell ->weapon.name, ENTITY_HEART_NAME);
			cell ->weapon.damage = heal_default.damage;
			cell ->weapon.range = heal_default.range;
			cell ->weapon.charge = heal_default.charge;
			cell ->representation = ENTITY_EMPTY;
			break;
		case ENTITY_POISON:
			cell ->weapon.type = ENTITY_POISON;
			strcpy(cell ->weapon.name, ENTITY_POISON_NAME);
			cell ->weapon.damage = poison_default.damage;
			cell ->weapon.range = poison_default.range;
			cell ->weapon.charge = poison_default.charge;
			cell ->representation = ENTITY_EMPTY;
			break;
		case ENTITY_BOMB:
			cell ->weapon.type = ENTITY_BOMB;
			strcpy(cell ->weapon.name, ENTITY_BOMB_NAME);
			cell ->weapon.damage = bomb_default.damage;
			cell ->weapon.range = bomb_default.range;
			cell ->weapon.charge = bomb_default.charge;
			cell ->representation = ENTITY_EMPTY;
			break;
		default:
			return -1;
	}

	int i;
	for (i = 0; i < UNITS_PER_CELL_MAX; i++) {
		cell ->units[i] = NULL;
	}
	
	return 0;
}




int d_level_pos(int x, int y) {
	return y * level_width + x;
}




int d_level_load(char *pathname) {
	// Open map file
	FILE *file;
	file = fopen(pathname, "r");
	if(file == NULL) {
		d_log("CRI: Map file not found or cannot be open");
		return -1;
	}
	
	// Read settings //
	
	int temporary_holder;
	
	if (read_setting(file, &level_width,
					 "level_width") < 0) {
		return -1;
	}
	if (read_setting(file, &level_height,
					 "level_height") < 0) {
		return -1;
	}
	if (read_setting(file, &level_active_health_reduction,
					 "level_active_health_reduction") < 0) {
		return -1;
	}
	if (read_setting(file, &level_passive_health_reduction,
					 "level_passive_health_reduction") < 0) {
		return -1;
	}
	if (read_setting(file, &level_passive_turns,
					 "level_passive_turns") < 0) {
		return -1;
	}
	
	fgetc(file);  // Empty line
	
	if (read_setting(file, &(heal_default.damage),
					 "heal_default.damage") < 0) {
		return -1;
	}
	if (read_setting(file, &(heal_default.range),
					 "heal_default.range") < 0) {
		return -1;
	}
	if (read_setting(file, &(heal_default.charge),
					 "heal_default.charge") < 0) {
		return -1;
	}
	
	if (read_setting(file, &(poison_default.damage),
					 "poison_default.damage") < 0) {
		return -1;
	}
	if (read_setting(file, &(poison_default.range),
					 "poison_default.range") < 0) {
		return -1;
	}
	if (read_setting(file, &(poison_default.charge),
					 "poison_default.charge") < 0) {
		return -1;
	}
	
	bomb_default.type = ENTITY_BOMB;
	strcpy(bomb_default.name, ENTITY_BOMB_NAME);
	if (read_setting(file, &(bomb_default.damage),
					 "bomb_default.damage") < 0) {
		return -1;
	}
	if (read_setting(file, &(bomb_default.range),
					 "bomb_default.range") < 0) {
		return -1;
	}
	if (read_setting(file, &(bomb_default.charge),
					 "bomb_default.charge") < 0) {
		return -1;
	}
	if (read_setting(file, &temporary_holder,
					 "bomb_default.delay") < 0) {
		return -1;
	}
	bomb_default.delay = (TICK_TYPE)temporary_holder;
	
	fgetc(file);  // Empty line
	fgetc(file);  // Empty line
	
	// Allocate memory and read cells //
	
	level = (CELL*)malloc(sizeof(CELL) * level_width * level_height);
	if (level == NULL) {
		d_log("CRI: Level allocation unsuccessful");
		fclose(file);
		return -1;
	}
	
	int x, y;
	char buffer[level_width];
	for (y = 0; y < level_height; y++) {
		if (fgets(buffer, level_width + 2, file) == NULL) {
			d_log("CRI: Invalid map: incorrect map size or shape");
			fclose(file);
			return -1;
		}
		for (x = 0; x < level_width; x++) {
			if (fill_cell(&(level[d_level_pos(x, y)]), buffer[x]) < 0) {
				d_log("CRI: Invalid map: unknown cell");
				fclose(file);
				return -1;
			}
		}
	}
	
	fclose(file);
	return 0;
}




int d_weapon_activate(WEAPON* weapon, int x, int y) {
	if ((weapon == NULL) || (x >= level_width) || (y >= level_height)) {
		return -1;
	}
	
	if ((weapon ->type == ENTITY_EMPTY) || (weapon ->charge <= 0)) {
		return 1;
	}
	
	if ((weapon ->type == ENTITY_HEART) || (weapon ->type == ENTITY_POISON) ||
			(weapon ->type == ENTITY_BOMB)) {
		int x_use, y_use;
		int i;
		for (y_use = (y - weapon ->range + 1);
			 y_use <= (y + weapon ->range - 1);
			 y_use++) {
			for (x_use = (x - weapon ->range + 1);
				 x_use <= (x + weapon ->range - 1);
				 x_use++) {
				if ((x_use >= level_width) || (x_use < 0) ||
						(y_use >= level_height) || (y_use < 0)) {
					continue;
				}
				if (level[d_level_pos(x_use, y_use)].representation ==
					ENTITY_WALL) {
					continue;
				}
				for (i = 0; i < UNITS_PER_CELL_MAX; i++) {
					if (level[d_level_pos(x_use, y_use)].units[i] == NULL) {
						break;
					}
					level[d_level_pos(x_use, y_use)].units[i] ->health -=
							weapon ->damage;
					if (level[d_level_pos(x_use, y_use)].units[i] ->health < 0) {
						level[d_level_pos(x_use, y_use)].units[i] ->health = 0;
					}
				}
			}
		}
	}
	else {
		return -1;
	}
	
	weapon ->charge -= 1;
	if (weapon ->charge == 0) {
		if (level[d_level_pos(x, y)].weapon.type != ENTITY_EMPTY) {
			level[d_level_pos(x, y)].weapon.type = ENTITY_EMPTY;
		}
	}
	return 0;
}




int d_unit_move(UNIT* unit, int x, int y) {
	// Argument checks
	if (unit == NULL) {
		return -1;
	}
	if ((x >= level_width) || (y >= level_height) || (x < 0) || (y < 0)) {
		return -1;
	}
	
	// Game checks
	if (unit ->health <= 0) {
		return 1;
	}
	if (unit ->next_action_tick > tick) {
		return 2;
	}
	if (level[d_level_pos(x, y)].representation == ENTITY_WALL) {
		return 3;
	}
	
	// Check if given cell has free space to hold a unit
	int new_i;
	for (new_i = 0; new_i < UNITS_PER_CELL_MAX; new_i++) {
		if (level[d_level_pos(x, y)].units[new_i] == NULL) {
			break;
		}
	}
	if (new_i == UNITS_PER_CELL_MAX) {
		return 3;
	}
	
	// Move unit
	{
		int prev_i;  // Player index at previous cell
		for (prev_i = 0; prev_i < UNITS_PER_CELL_MAX; prev_i++) {
			if (level[d_level_pos(unit->x, unit->y)].units[prev_i] == unit) {
				break;
			}
		}
		
		int prev_li;  // Last (another) player index at previous cell
		for (prev_li = UNITS_PER_CELL_MAX - 1; prev_li >= 0; prev_li--) {
			if (level[d_level_pos(unit->x, unit->y)].units[prev_li] != NULL) {
				break;
			}
		}
		
		if ((prev_i == UNITS_PER_CELL_MAX) || (prev_li < 0)) {  // Move error
			return -1;
		}
		
		if (prev_li == prev_i) {  // Given unit is the last one
			level[d_level_pos(unit->x, unit->y)].units[prev_i] = NULL;
		}
		else {  // Make given unit the last one and then remove it
			level[d_level_pos(unit->x, unit->y)].units[prev_i] =
					level[d_level_pos(unit->x, unit->y)].units[prev_li];
			level[d_level_pos(unit->x, unit->y)].units[prev_li] = NULL;
		}
		
		level[d_level_pos(x, y)].units[new_i] = unit;
		unit ->x = x;
		unit ->y = y;
		unit ->next_action_tick = tick + unit ->move_delay;
	}
	
	// Process side effects
	d_weapon_activate(&(level[d_level_pos(x, y)].weapon), x, y);
	if (unit ->health <= 0) {
		unit ->health = 0;
		return 1;
	}
	
	return 0;
}




int d_unit_use_weapon(UNIT* unit) {
	// Checks
	if (unit == NULL) {
		return -1;
	}
	if ((unit ->weapon.type == ENTITY_EMPTY) ||
		(unit ->next_action_tick > tick) ||
		(level[d_level_pos(unit->x, unit->y)].weapon.type != ENTITY_EMPTY)) {
		return 2;
	}
	if (unit ->weapon.charge <= 0) {
		return 3;
	}
	
	switch (unit ->weapon.type) {
		case ENTITY_BOMB:
			level[d_level_pos(unit->x, unit->y)].weapon.type =
					unit ->weapon.type;
			strcpy(level[d_level_pos(unit->x, unit->y)].weapon.name,
				   unit ->weapon.name);
			level[d_level_pos(unit->x, unit->y)].weapon.damage =
					unit ->weapon.damage;
			level[d_level_pos(unit->x, unit->y)].weapon.range =
					unit ->weapon.range;
			level[d_level_pos(unit->x, unit->y)].weapon.charge =
					1;
			unit ->weapon.charge -= 1;
			break;
		case ENTITY_HEART:
		case ENTITY_POISON:
			if (d_weapon_activate(&(unit ->weapon), unit ->x, unit ->y) < 0) {
				return -1;
			}
			break;
		default:
			return -1;
	}
	unit ->next_action_tick = tick + unit ->weapon.delay;
	
	if (unit ->health <= 0) {
		unit ->health = 0;
		return 1;
	}
	
	return 0;
}




int d_unit_get_position(int* x, int* y) {
	srand((unsigned int)time(NULL));
	int x_rand = rand() % ((level_width * 2) / 3);
	int y_rand = rand() % (level_height - 1);
	
	int x_curr, y_curr;
	int is_position_found = 0;
	for (y_curr = y_rand; y_curr < level_height; y_curr++) {
		for (x_curr = x_rand; x_curr < level_width; x_curr++) {
			CELL* cell_ptr = level + d_level_pos(x_curr, y_curr);
			if ((cell_ptr ->representation == ENTITY_EMPTY) &&
					(cell_ptr ->units[0] == NULL) &&
					(cell_ptr ->weapon.type == ENTITY_EMPTY)) {
				*x = x_curr;
				*y = y_curr;
				is_position_found = 1;
			}
			if (is_position_found == 1) {
				break;
			}
		}
		if (is_position_found == 1) {
			break;
		}
	}
	
	if (is_position_found == 1) {
		return 0;
	}
	
	for (y_curr = 0; y_curr < level_height; y_curr++) {
		for (x_curr = 0; x_curr < level_width; x_curr++) {
			CELL* cell_ptr = level + d_level_pos(x_curr, y_curr);
			if ((cell_ptr ->representation == ENTITY_EMPTY) &&
				(cell_ptr ->units[0] == NULL) &&
				(cell_ptr ->weapon.type == ENTITY_EMPTY)) {
				*x = x_curr;
				*y = y_curr;
				is_position_found = 1;
			}
			if (is_position_found == 1) {
				break;
			}
		}
		if (is_position_found == 1) {
			break;
		}
	}
	
	if (is_position_found == 1) {
		return 0;
	}
	
	return -1;
}




int d_unit_process_command(char cmd, UNIT* unit) {
	// Checks
	if (unit == NULL) {
		return -1;
	}
	
	// Process command
	
	char health_reduction_type;  // Type of planned health reduction ('p' / 'a')
	int exec_result;
	
	if (cmd == CMD_NONE) {
		health_reduction_type = 'p';
	}
	else if (cmd == CMD_W) {
		if (unit ->y - 1 < 0) {
			health_reduction_type = 'p';
		}
		else {
			exec_result = d_unit_move(unit, unit ->x, unit ->y - 1);
			if (exec_result == 0) {
				health_reduction_type = 'a';
			}
			else if (exec_result > 0) {
				health_reduction_type = 'p';
			}
			else {
				return -1;
			}
		}
	}
	else if (cmd == CMD_A) {
		if (unit ->x - 1 < 0) {
			health_reduction_type = 'p';
		}
		else {
			exec_result = d_unit_move(unit, unit ->x - 1, unit ->y);
			if (exec_result == 0) {
				health_reduction_type = 'a';
			}
			else if (exec_result > 0) {
				health_reduction_type = 'p';
			}
			else {
				return -1;
			}
		}
	}
	else if (cmd == CMD_S) {
		if (unit ->y + 1 >= level_height) {
			health_reduction_type = 'p';
		}
		else {
			exec_result = d_unit_move(unit, unit ->x, unit ->y + 1);
			if (exec_result == 0) {
				health_reduction_type = 'a';
			}
			else if (exec_result > 0) {
				health_reduction_type = 'p';
			}
			else {
				return -1;
			}
		}
	}
	else if (cmd == CMD_D) {
		if (unit ->x + 1 >= level_width) {
			health_reduction_type = 'p';
		}
		else {
			exec_result = d_unit_move(unit, unit ->x + 1, unit ->y);
			if (exec_result == 0) {
				health_reduction_type = 'a';
			}
			else if (exec_result > 0) {
				health_reduction_type = 'p';
			}
			else {
				return -1;
			}
		}
	}
	else if (cmd == CMD_WEAPON) {
		exec_result = d_unit_use_weapon(unit);
		if (exec_result == 0) {
			health_reduction_type = 'a';
		}
		else if (exec_result > 0) {
			health_reduction_type = 'p';
		}
		else {
			return -1;
		}
	}
	else if (cmd == CMD_QUIT){
		unit ->health = 0;
		health_reduction_type = 'p';
	}
	else {
		return -1;  // Unknown command
	}
	
	// Process health reduction
	if (health_reduction_type == 'p') {
		unit ->health -= level_passive_health_reduction;
	}
	else {
		unit ->health -= level_active_health_reduction;
	}
	
	// Remove dead unit from the map (but leave its position unchanged)
	if (unit ->health <= 0) {
		unit ->health = 0;
		int i;  // Dead unit position in level[...].units array
		for (i = 0; i < UNITS_PER_CELL_MAX; i++) {
			if (level[d_level_pos(unit->x, unit->y)].units[i] == unit) {
				break;
			}
		}
		
		int i_last;  // Last unit position in level[...].units array
		for (i_last = UNITS_PER_CELL_MAX - 1; i >= 0; i--) {
			if (level[d_level_pos(unit->x, unit->y)].units[i_last] != NULL) {
				break;
			}
		}
		
		if ((i_last < 0) || (i == UNITS_PER_CELL_MAX)) {
			return -1;
		}
		
		if (i_last == i) {  // Dead unit is the last one
			level[d_level_pos(unit->x, unit->y)].units[i] = NULL;
		}
		else {  // Make dead unit the last one and then remove it
			level[d_level_pos(unit->x, unit->y)].units[i] =
					level[d_level_pos(unit->x, unit->y)].units[i_last];
			level[d_level_pos(unit->x, unit->y)].units[i_last] = NULL;
		}
		
		return 1;
	}
	
	return 0;
}




int d_game_update(void) {
	char log_message[LENGTH_LOG_MESSAGE];
	
	// Checks
	if ((units_total <= 0) || (level == NULL) || (units == NULL)) {
		return -1;
	}
	
	// Processing
	int i;
	int exec_result;
	for (i = 0; i < units_total; i++) {
		exec_result = d_unit_process_command(units_cmd[i], &(units[i]));
		if (exec_result == 0) {
			if (units_cmd[i] == CMD_WEAPON) {
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t used weapon",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
			else if (units_cmd[i] != CMD_NONE) {
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t moved",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
		}
		else if (exec_result > 0) {
			if (units_cmd[i] == CMD_QUIT) {
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t quit",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
			else if (units_cmd[i] == CMD_WEAPON) {
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t used weapon and died",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
			else if (units_cmd[i] != CMD_NONE){
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t died in action",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
			else {
				sprintf(log_message,
						"GAM: #%04d (%04d, %04d)\t died of laziness",
						i, units[i].x, units[i].y);
				d_log(log_message);
			}
		}
		else {
			sprintf(log_message,
					"ERR: #%04d (%04d, %04d)\t caused a processing error",
					i, units[i].x, units[i].y);
			d_log(log_message);
		}
		units_cmd[i] = CMD_NONE;
	}
	
	return 0;
}




int d_game_refresh(void) {
	// Checks
	if ((units_total <= 0) || (level == NULL) || (units == NULL)) {
		return -2;
	}
	
	// Count number of units and enemies alive
	int i;
	int total_players_alive = 0;
	int total_enemies_alive = 0;
	for (i = 0; i < units_total; i++) {
		if (units[i].health > 0) {
			if (units[i].type == ENTITY_PLAYER) {
				total_players_alive += 1;
			}
			else {  // if (units[i].type == ENTITY_ENEMY)
				total_enemies_alive += 1;
			}
		}
	}
	
	// Compute exit code
	int exit_code;
	if (total_players_alive > 1) {
		exit_code = 0;
	}
	else if (total_players_alive == 1) {
		if (total_enemies_alive > 0) {
			exit_code = 0;
		}
		else {
			exit_code = 1;
		}
	}
	else {
		exit_code = -1;
	}
	

	tick += 1;
	
	return exit_code;
}




void d_game_shutdown(void) {
	free(level);
	free(units);
	free(units_cmd);
}



