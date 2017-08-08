#include "dgame.h"




int d_weapon_operate(WEAPON* weapon, UNIT* unit) {
	if ((weapon == NULL) || (unit == NULL)) {
		return -1;
	}
	if (weapon ->charge <= 0) {
		return 1;
	}
	weapon ->charge -= 1;
	unit ->health -= weapon ->damage;
	if (unit ->health < 0) {
		unit ->health = 0;
	}
	return 0;
}


void d_level_load(char *pathname) {
	FILE *file;
	int i,j;
	file = fopen(pathname, "r");
	if(file == NULL)
	{
		printf("can't open file\n");
	}
	char string[40];
	if (fscanf (file, "%s%s%d", string, string, &level_active_health_reduction) == EOF) {
		printf("can't read value of level_active_health_reduction\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &level_passive_health_reduction) == EOF) {
		printf("can't read value of level_passive_health_reduction\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &DWEAPON_heal_damage) == EOF) {
		printf("can't read value of DWEAPON_heal_damage\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &DWEAPON_poison_damage) == EOF) {
		printf("can't read value of DWEAPON_poison_damage\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &DWEAPON_bomb_range) == EOF) {
		printf("can't read value of DWEAPON_bomb_range\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &DWEAPON_bomb_damage) == EOF) {
		printf("can't read value of DWEAPON_bomb_damage\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &level_height) == EOF) {
		printf("can't read value of level_height\n");
		exit(1);
	}
	if (fscanf (file, "%s%s%d", string, string, &level_width) == EOF) {
		printf("can't read value of level_width\n");
		exit(1);
	}
	fgets(string,level_width,file); // cчитываю пустую строку
	levelData0 = (char**)malloc(level_height * sizeof(char*));
	for (i = 0; i < level_height; ++i) {
		levelData0[i] = (char*)malloc(level_width * sizeof(char));
	}
	for (i = 0; i < level_height; ++i) {
		char str[level_width];
		if (fgets(str, level_width + 2, file) == NULL) {
			printf("error");
			exit(1);
		}
		for (j = 0; j < level_width; ++j) {
			levelData0[i][j] = str[j];
		}
	}
	fclose(file);
}



const int PLAYERS_MAX = 1;


/////////////////////////////////////
// Logics variables
bool isGameActive = false;
clock_t clockLastFrame = 0;

int framesCounter = 0;
float framesTimeCounter = 0;
int fps = 0;

int units_total = 0;
int heroIndex = 0;

bool InsertUnitOnMap(int *r, int *c) {
	int i, j, r1, c1;
	for (i = -1; i <= 1; ++i) {
		for (j = -1; j <= 1; ++j) {
			r1 = *r + i;
			c1 = *c + j;
			if (((r1 >= 0) && (r1 < level_height)) &&
				((c1 >= 0) && (c1 < level_width))) {
				if (level[r1][c1] == ENTITY_EMPTY) {
					level[r1][c1] = ENTITY_PLAYER;
					*r = r1;
					*c = c1;
					return true;
				}
			}
		}
	}
	return false;
}

void Initialize(char *pathname) {
	//read levelData0 and others
	d_level_load(pathname);
	
	// Set clockLastFrame start value
	clockLastFrame = clock();
	
	units = (UNIT *) malloc(units_total * sizeof(UNIT));
	level = (char **) malloc(level_height * sizeof(char *));
	for (int i = 0; i < level_height; ++i) {
		level[i] = (char *) malloc(level_width * sizeof(char));
	}
	//units_total = 0; определяем в main-е
	
	int r, c;
	// Load level
	for (r = 0; r < level_height; r++) {
		for (c = 0; c < level_width; c++) {
			unsigned char cellSymbol = levelData0[r][c];
			
			level[r][c] = cellSymbol;
		}
	}
	int i, row, column;
	
	units_players = units_total;
	UnitType unitType;
	for (i = 0; i < units_total; ++i) {
		unitType = UnitType_Hero;
		do {
			row = rand() % level_height;
			column = rand() % level_width;
		} while (InsertUnitOnMap(&row, &column) == false);
		units[i].type = unitType;
		units[i].id = i;
		units[i].y = (float) (row);
		units[i].x = (float) (column);
		units[i].health = GetUnitDefaultHealth(unitType);
		units[i].speed_x = 0.0;
		units[i].speed_y = 0.0;
		units[i].order_x = UnitOrder_None;
		units[i].order_y = UnitOrder_None;
		units[i].weapon = GetUnitDefaultWeapon(unitType);
		units[i].weapon_charge = GetUnitDefaultCountOfCharge();
	}
	heroIndex = 0;
	isGameActive = true;  // Костя просил убрать
}

int d_game_update() {
	// Calculate delta time
	clock_t clockNow = clock();
	clock_t deltaClock = clockNow - clockLastFrame;
	float deltaTime = (float) (deltaClock) / CLOCKS_PER_SEC;
	clockLastFrame = clockNow;
	
	
	// Calculate FPS
	framesCounter++;
	framesTimeCounter += deltaTime;
	if (framesTimeCounter >= 1.0) {
		framesTimeCounter -= 1.0;
		fps = framesCounter;
		framesCounter = 0;
	}
	
	int i, c;
	// Hero control
	for (i = 0; i < units_total; ++i) {
		c = units[i].last_action_stamp;
		units[i].last_action_stamp = -1; // хорошо было бы иметь атомики
		if (c == 'w') {
			units[i].order_y = UnitOrder_Backward;
		}
		if (c == 's') {
			units[i].order_y = UnitOrder_Forward;
		}
		
		if (c == 'a') {
			units[i].order_x = UnitOrder_Backward;
		}
		if (c == 'd') {
			units[i].order_x = UnitOrder_Forward;
		}
		if (c == 'b') {
			SetBomb(&units[i]);
		}
		if (c == 'q') {
			isGameActive = false;
		}
	}
	
	
	// d_game_update all units
	for (int u = 0; u < units_total; u++) {
		d_unit_process_command(NULL, &units[u]);
		//record_in_statistics(units[u].id, units[u].health);
	}
	
	
	// Hero dead
	if (units_players <= 1)
		isGameActive = false;
}

void d_game_shutdown() {
	int i;
	for (i = 0; i < level_height; ++i) {
		free(levelData0[i]);
		free(level[i]);
	}
	free(levelData0);
	free(level);
	free(units);
}

bool d_unit_move(UNIT *unit, int x, int y) {
	// Ignore dead units
	if (unit->health <= 0) {
		return false;
	}
	
	int row = (int) (y);
	int column = (int) (x);
	int oldRow = (int) (unit->y);
	int oldColumn = (int) (unit->x);
	
	unsigned char unitSymbol = level[oldRow][oldColumn];
	unsigned char destinationCellSymbol = level[row][column];
	bool canMoveToCell = false;
	
	// All units actions
	
	// Empty cell
	if (destinationCellSymbol == ENTITY_EMPTY) {
		canMoveToCell = true;
	}
	// Units cells
	if (destinationCellSymbol == ENTITY_PLAYER) {
		UnitType destinationUnitType = GetUnitTypeFromCell(
				destinationCellSymbol);
		
		// Find enemy unit struct
		for (int u = 0; u < units_total; u++) {
			// Ignore dead units
			if (units[u].health <= 0)
				continue;
			
			// Ignore yourself
			if (&units[u] == unit)
				continue;
			
			if ((int) (units[u].y) == row &&
				(int) (units[u].x) == column) {
				// Calculate weapon DWEAPON_bomb_damage
				int damage = GetWeaponDamage(WeaponType_Fist);
				
				// Deal DWEAPON_bomb_damage
				units[u].health -= damage;
				
				// If enemy unit die
				if (units[u].health <= 0) {
					level[row][column] = ENTITY_EMPTY;
					units_players--;
				}
				
				break;
			}
		}
	}
	
	// Only hero actions
	if (unit->type == UnitType_Hero) {
		// Heart
		if (destinationCellSymbol == ENTITY_HEART) {
			canMoveToCell = true;
			unit->health += DWEAPON_heal_damage;
		}
		// Poison
		if (destinationCellSymbol == ENTITY_POISON) {
			canMoveToCell = true;
			unit->health -= DWEAPON_poison_damage;
			if (unit->health <= 0) {
				units_players--;
			}
		}
	}
	
	if (canMoveToCell) {
		// Remove unit symbol from previous position
		level[oldRow][oldColumn] = ENTITY_EMPTY;
		
		// Set new hero position
		unit->x = x;
		unit->y = y;
		
		// Set hero symbol to new position
		level[row][column] = unitSymbol;
	}
	
	return canMoveToCell;
}

void SetBomb(UNIT *pointerToUnitData) {
	pointerToUnitData->weapon_charge -= 1;
	for (int u = 0; u < units_total; u++) {
		// Ignore dead units
		if (units[u].health <= 0)
			continue;
		
		// Ignore yourself
		if (&units[u] == pointerToUnitData)
			continue;
		
		int row = (int) (pointerToUnitData->y);
		int column = (int) (pointerToUnitData->x);
		int max, unit_damage;
		if ((int) (units[u].y) <= row + DWEAPON_bomb_range &&
			(int) (units[u].y) >= row - DWEAPON_bomb_range &&
			(int) (units[u].x) <= column + DWEAPON_bomb_range &&
			(int) (units[u].x) >= column - DWEAPON_bomb_range) {
			max = abs((int) (units[u].y) - row);
			if (abs((int) (units[u].x) - column) > max) {
				max = abs((int) (units[u].x) - column);
			}
			unit_damage = (int) (DWEAPON_bomb_damage / DWEAPON_bomb_range *
								 (DWEAPON_bomb_range + 1 - max));
			if (unit_damage > 0) {
				units[u].health -= unit_damage; // пока без препятствий
			}
			if (units[u].health <= 0) {
				units_players--;
			}
		}
	}
	
}

int d_unit_process_command(char *cmd, UNIT *unit) {
	// Unit row and column
	int row = (int) (unit->y);
	int column = (int) (unit->x);
	
	
	// X Order
	if (unit->order_x == UnitOrder_Backward) {
		unit->speed_x = -GetUnitSpeed(unit->type);
	} else {
		if (unit->order_x == UnitOrder_Forward) {
			unit->speed_x = GetUnitSpeed(unit->type);
		} else {
			unit->speed_x = 0;
		}
	}
	
	// Y Order
	if (unit->order_y == UnitOrder_Backward) {
		unit->speed_y = -GetUnitSpeed(unit->type);
	} else {
		if (unit->order_y == UnitOrder_Forward) {
			unit->speed_y = GetUnitSpeed(unit->type);
		} else {
			unit->speed_y = 0;
		}
	}
	
	
	// New position
	float deltaY = unit->speed_y * deltaTime;
	float deltaX = unit->speed_x * deltaTime;
	float newY = unit->y + deltaY;
	float newX = unit->x + deltaX;
	int newRow = (int) (newY);
	int newColumn = (int) (newX);
	
	
	// Y( row ) step
	if (newRow != row) {
		// If unit can go to cell
		if (newRow < 0) {
			unit->y = row + cellBeginValue;
		} else {
			if (newRow >= level_height) {
				unit->y = row + cellEndValue;
			} else {
				if (d_unit_move(unit, unit->x, newY) ==
					false) {
					// Can not move cell down
					if (newRow > row) {
						unit->y = row + cellEndValue;
					} else {
						unit->y = row + cellBeginValue;
					}
				}
			}
		}
	} else {
		unit->y = newY;
	}
	
	// X( column ) step
	if (newColumn != column) {
		// If unit can go to cell
		if (newColumn < 0) {
			unit->x = column + cellBeginValue;
		} else {
			if (newColumn >= level_width) {
				unit->x = column + cellEndValue;
			} else {
				if (d_unit_move(unit, newX, unit->y) ==
					false) {
					// Can not move cell right
					if (newColumn > column) {
						unit->x = column + cellEndValue;
					} else {
						unit->x = column + cellBeginValue;
					}
				}
			}
		}
	} else {
		unit->x = newX;
	}
	unit->order_x = UnitOrder_None;
	unit->order_y = UnitOrder_None;
	unit->speed_x = 0;
	unit->speed_y = 0;
}
