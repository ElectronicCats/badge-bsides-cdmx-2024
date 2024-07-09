#ifndef ARM_WRESTLING_H
#define ARM_WRESTLING_H

#include "esp_timer.h"
#include "keyboard_module.h"

#define MAX_ARM_WRESTLING_PLAYERS 2

typedef enum {
  UPDATE_PLAYER_DATA_CMD = 10,
  UPDATE_GAME_DATA_CMD,
  GAME_OVER_CMD,
  STOP_ARM_WRESTLING_CMD
} arm_wrestling_commands_t;

typedef enum { UPDATE_GAME_EVENT } arm_wrestling_events_t;

typedef struct {
  uint16_t strength;
} player_data_t;

typedef struct {
  player_data_t players_data[MAX_ARM_WRESTLING_PLAYERS];
  int16_t arm_position;  // -1000 to 1000, 0 is center
} game_data_t;

typedef struct {
  uint8_t cmd;
  player_data_t player_data;
} player_data_cmd_t;

typedef struct {
  uint8_t cmd;
  game_data_t game_data;
} game_data_cmd_t;

typedef struct {
  uint8_t cmd;
} stop_game_cmd_t;

typedef struct {
  uint8_t cmd;
} game_over_cmd_t;

player_data_t* me;
game_data_t game_instance;

extern uint8_t arm_wrestling_player_id;

void arm_wrestling_init();
void arm_wrestling_input(button_event_t button_pressed);

#endif  // ARM_WRESTLING_H
