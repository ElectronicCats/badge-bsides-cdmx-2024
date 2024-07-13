#ifndef SPEED_BAG_GAME_H
#define SPEED_BAG_GAME_H

#include "esp_timer.h"
#include "keyboard_module.h"

#define MAX_BAG_GAME_PLAYERS 5

typedef enum {
  SPEED_BAG_UPDATE_PLAYER_DATA_CMD = 10,
  SPEED_BAG_UPDATE_GAME_DATA_CMD,
  SPEED_BAG_GAME_OVER_CMD,
  SPEED_BAG_STOP_GAME_CMD
} speed_bag_speed_bag_game_commands_t;
typedef enum { SPEED_BAG_UPDATE_GAME_EVENT } speed_bag_speed_bag_game_events_t;

typedef enum {
  SPEED_BAG_TIRED_STATE = 0,
  SPEED_BAG_NORMAL_STATE = 100,
  SPEED_BAG_ANGRY_STATE = 200,
  SPEED_BAG_FUIRIOUS_STATE = 255,
} speed_bag_player_state_t;

typedef struct {
  uint8_t strenght;
  int winner_id;
} speed_bag_player_data_t;

typedef struct {
  speed_bag_player_data_t players_data[MAX_BAG_GAME_PLAYERS];
  int16_t bag_bar;
} speed_bag_game_data_t;

typedef struct {
  uint8_t cmd;
  speed_bag_player_data_t player_data;
} speed_bag_player_data_cmd_t;

typedef struct {
  uint8_t cmd;
  speed_bag_game_data_t game_data;
} speed_bag_game_data_cmd_t;

typedef struct {
  uint8_t cmd;
} speed_bag_stop_game_cmd_t;

typedef struct {
  uint8_t cmd;
  int winner_id;
} speed_bag_game_over_cmd_t;

speed_bag_player_data_t* speed_bag_main_player;
speed_bag_game_data_t speed_bag_game_instance;

uint8_t speed_bag_player_id;

void speed_bag_game_init();
void speed_bag_game_input(button_event_t button_pressed);

#endif  // SPEED_BAG_GAME_H
