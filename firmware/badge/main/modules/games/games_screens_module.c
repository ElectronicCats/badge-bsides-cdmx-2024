#include "games_screens_module.h"
#include <string.h>
#include "lobby_manager.h"
#include "oled_screen.h"
#include "rope_game.h"

#define BAR_HEIGHT 8
#define BAR_WIDTH  112

uint8_t bar_bitmap[BAR_HEIGHT][BAR_WIDTH / 8];

void show_host_state() {}

void show_available_games() {
  oled_screen_display_text(" ^     >     *", 4, 2, OLED_DISPLAY_NORMAL);
  oled_screen_display_text("1vs1  2vs2   5P", 0, 3, OLED_DISPLAY_NORMAL);
}

void show_client_state() {
  oled_screen_clear();
  char* player_idx = (char*) malloc(20);
  sprintf(player_idx, " Client Mode: P%d", my_client_id);
  oled_screen_display_text(player_idx, 0, 1, OLED_DISPLAY_NORMAL);
  free(player_idx);
}

void show_clients() {
  //   oled_screen_display_text(" P0  P1   P2  P3", 0, 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text(" SELECT A GAME ", 0, 0, OLED_DISPLAY_INVERT);
  char* online_players = (char*) malloc(20);
  bool online[MAX_PLAYERS_NUM - 1] = {0, 0, 0, 0};
  for (uint8_t i = 1; i < MAX_PLAYERS_NUM; i++) {
    online[i - 1] = players[i].online;
  }
  sprintf(online_players, " %d   %d    %d   %d", online[0], online[1],
          online[2], online[3]);
  oled_screen_display_text(online_players, 4, 1, OLED_DISPLAY_NORMAL);
  free(online_players);
}

void show_badge_unconnected() {
  oled_screen_clear();
  oled_screen_display_text("Connect your badges", 0, 1, OLED_DISPLAY_NORMAL);
}

void games_screens_module_show_lobby_state(uint8_t state) {
  switch (state) {
    case HOST_STATE:
      show_host_state();
      break;
    case CLIENT_STATE:
      show_client_state();
      break;
    case SHOW_CLIENTS:
      oled_screen_clear();
      show_available_games();
      show_clients();
      break;
    case SHOW_UNCONNECTED:
      show_badge_unconnected();
      break;
    default:
      break;
  }
}
//////////////////////////////////////////////////////////////////////////////////
void update_bar(uint8_t value) {
  uint8_t active_cols = (uint32_t) value * BAR_WIDTH / 255;
  memset(bar_bitmap, 0, sizeof(bar_bitmap));
  for (int y = 0; y < BAR_HEIGHT; y++) {
    if (y == 1 || y == 6)
      continue;
    for (int x = 0; x < active_cols; x++) {
      bar_bitmap[y][x / 8] |= (1 << (7 - (x % 8)));
    }
  }
}
void rope_game_show_game_data() {
  oled_screen_clear();
  char* str = (char*) malloc(16);
  for (int8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    sprintf(str, "P%d", i + 1);
    oled_screen_display_text(
        str, 0, i,
        i == my_client_id ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);
    update_bar(game_instance.players_data[i].strenght);
    oled_screen_display_bitmap(bar_bitmap, 16, i * 8, BAR_WIDTH, BAR_HEIGHT,
                               OLED_DISPLAY_NORMAL);
  }
  free(str);
}

void games_screens_module_show_game_over(bool winner) {
  oled_screen_clear();
  char* str = (char*) malloc(16);
  sprintf(str, "Team %d won\n", winner + 1);
  oled_screen_display_text(str, 0, 1, OLED_DISPLAY_NORMAL);
  printf("Team %d won\n", winner + 1);
  free(str);
}
void games_screens_module_show_rope_game_event(rope_game_events_t event) {
  switch (event) {
    case UPDATE_GAME_EVENT:
      rope_game_show_game_data();
      break;
    default:
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////////
void show_not_enought_players() {
  oled_screen_clear();
  oled_screen_display_text("NOT ENOUGHT", 0, 0, OLED_DISPLAY_NORMAL);
  char* str = (char*) malloc(16);
  sprintf(str, "BADGES %d/4", get_clients_count());
  oled_screen_display_text(str, 0, 1, OLED_DISPLAY_NORMAL);
  free(str);
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void games_screens_module_show_games_module_event(games_module_events_t event) {
  lobby_manager_set_display_status_cb(NULL);
  switch (event) {
    case NOT_ENOUGHT_BADGES_EVENT:
      show_not_enought_players();
      break;
    default:
      break;
  }
  lobby_manager_set_display_status_cb(games_screens_module_show_lobby_state);
}
//////////////////////////////////////////////////////////////////////////////////
