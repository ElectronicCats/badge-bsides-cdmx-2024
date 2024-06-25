#ifndef ROPE_GAME_H
#define ROPE_GAME_H

#include "esp_timer.h"
#include "keyboard_module.h"

#define MAX_ROPE_GAME_PLAYERS 4

typedef enum { UPDATE_PLAYER_DATA = 10, UPDATE_GAME_DATA } rope_game_commands_t;
typedef enum { UPDATE_GAME_EVENT } rope_game_events_t;

typedef enum {
  TIRED_STATE = 0,
  NORMAL_STATE = 100,
  ANGRY_STATE = 200,
  FUIRIOUS_STATE = 255,
} player_state_t;

typedef struct {
  uint8_t strenght;
} player_data_t;

typedef struct {
  player_data_t players_data[MAX_ROPE_GAME_PLAYERS];
  int16_t rope_bar;
} game_data_t;

typedef struct {
  uint8_t cmd;
  player_data_t player_data;
} player_data_msg_t;

typedef struct {
  uint8_t cmd;
  game_data_t game_data;
} game_data_msg_t;

player_data_t* me;
game_data_t game_instance;

void rope_game_init();
void rope_game_input(button_event_t button_pressed);

#endif  // ROPE_GAME_H
