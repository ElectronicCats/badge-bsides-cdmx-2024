#pragma once
#include <stdio.h>
#define TAG_GAMES_MODULE "games_module:Main"

typedef enum { RAUL_GAME, ROPE_GAME, KEVIN_GAME } games_id_t;
typedef enum { NOT_ENOUGHT_BADGES_EVENT } games_module_events_t;

typedef enum {
  START_GAME = 20,
} games_module_cmds_t;

typedef struct {
  uint8_t cmd;
  uint8_t game_id;
} start_game_cmd_t;

void games_module_begin();
void games_module_setup();
