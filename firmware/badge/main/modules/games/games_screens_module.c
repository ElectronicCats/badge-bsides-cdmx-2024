#include "games_screens_module.h"
#include "lobby_manager.h"
#include "oled_screen.h"

void show_host_state() {}

void show_available_games() {
  oled_screen_display_text(" ^     >     *", 4, 2, OLED_DISPLAY_NORMAL);
  oled_screen_display_text("1vs1  2vs2   5P", 0, 3, OLED_DISPLAY_NORMAL);
}

void show_client_state() {
  oled_screen_clear();
  char* player_idx = (char*) malloc(20);
  sprintf(player_idx, " Client Mode: P%d", my_player_id);
  oled_screen_display_text(player_idx, 0, 1, OLED_DISPLAY_NORMAL);
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

void games_screens_module_show_state(uint8_t state) {
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
