#pragma once
#include <stdio.h>
#include "games_module.h"
#include "rope_game.h"
#include "speed_bag_game.h"

#define TAG_GAMES_MODULE "games_screens_module:Main"

void games_screens_module_show_lobby_state(uint8_t state);
void games_screens_module_show_rope_game_event(rope_game_events_t event);
void games_screens_module_show_games_module_event(games_module_events_t event);
void games_screens_module_show_game_over(bool winner, bool team);
void games_screen_module_show_game_over_speed(int winner);
void games_screens_module_show_speed_bag_game_event(
    speed_bag_speed_bag_game_events_t event);
