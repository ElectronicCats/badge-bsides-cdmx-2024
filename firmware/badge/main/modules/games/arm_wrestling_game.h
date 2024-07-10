#ifndef ARM_WRESTLING_H
#define ARM_WRESTLING_H

#include "esp_timer.h"
#include "keyboard_module.h"

#define MAX_ARM_WRESTLING_PLAYERS 2

typedef enum {
  WUPDATE_PLAYER_DATA_CMD = 10,
  WUPDATE_GAME_DATA_CMD,
  WGAME_OVER_CMD,
  WSTOP_ARM_WRESTLING_CMD
} arm_wrestling_commands_t;

typedef enum { WUPDATE_GAME_EVENT } arm_wrestling_events_t;

typedef struct {
  uint16_t strength;
} wplayer_data_t;

typedef struct {
  wplayer_data_t players_data[MAX_ARM_WRESTLING_PLAYERS];
  int16_t arm_position;  // -1000 to 1000, 0 is center
} wgame_data_t;

typedef struct {
  uint8_t cmd;
  wplayer_data_t player_data;
} wplayer_data_cmd_t;

typedef struct {
  uint8_t cmd;
  wgame_data_t game_data;
} wgame_data_cmd_t;

typedef struct {
  uint8_t cmd;
} wstop_game_cmd_t;

typedef struct {
  uint8_t cmd;
} wgame_over_cmd_t;

wplayer_data_t* wme;
wgame_data_t wgame_instance;

extern uint8_t arm_wrestling_player_id;

void arm_wrestling_init();
void arm_wrestling_input(button_event_t button_pressed);

#endif  // ARM_WRESTLING_H
