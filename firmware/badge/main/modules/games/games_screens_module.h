#pragma once
#include <stdio.h>
#include "rope_game.h"

#define TAG_GAMES_MODULE "games_screens_module:Main"

void games_screens_module_show_lobby_state(uint8_t state);
void games_screens_module_show_rope_game_event(rope_game_events_t event);
