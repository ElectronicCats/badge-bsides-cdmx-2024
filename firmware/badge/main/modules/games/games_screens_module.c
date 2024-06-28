#include "games_screens_module.h"
#include <string.h>
#include "games_bitmaps.h"
#include "lobby_manager.h"
#include "oled_screen.h"
#include "rope_game.h"

#define BAR_HEIGHT 8
#define BAR_WIDTH  64

uint8_t bar_bitmap[BAR_HEIGHT][BAR_WIDTH / 8];

void show_host_state() {}

void show_scanning_dots(uint8_t x, uint8_t page, uint8_t dots_num) {
  static uint8_t dots = 0;
  dots = ++dots > dots_num ? 0 : dots;
  for (int i = 0; i < dots; i++) {
    oled_screen_display_text(".", x + (i * 8), page, OLED_DISPLAY_NORMAL);
  }
}

void show_client_state() {
  oled_screen_clear();
  char* player_idx = (char*) malloc(20);
  sprintf(player_idx, " Client Mode: P%d", my_client_id + 1);
  oled_screen_display_text(player_idx, 0, 1, OLED_DISPLAY_NORMAL);
  free(player_idx);
}

void show_available_game() {
  uint8_t players_count = get_clients_count();
  static bool frame = 0;
  frame = !frame;
  switch (players_count) {
    case 2:
      oled_screen_display_text("RAUL GAME", 4, 3, OLED_DISPLAY_NORMAL);
      break;
    case 3:
      oled_screen_display_text("Waiting 3/4", 4, 3, OLED_DISPLAY_NORMAL);
      show_scanning_dots(95, 3, 3);
      return;
    case 4:
      oled_screen_display_text("ROPE GAME", 4, 3, OLED_DISPLAY_NORMAL);
      break;
    case 5:
      oled_screen_display_text("KEVIN GAME", 4, 3, OLED_DISPLAY_NORMAL);
      break;
    default:
      oled_screen_display_bitmap(badge_connection_bmp_arr[frame], 32, 8, 64, 16,
                                 OLED_DISPLAY_NORMAL);
      oled_screen_display_text("Waiting 1/2", 4, 3, OLED_DISPLAY_NORMAL);
      show_scanning_dots(95, 3, 3);
      return;
      break;
  }
  oled_screen_display_bitmap(enter_button_bmp, 112, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
}

void show_clients() {
  oled_screen_display_text_center("Connect Badges", 0, OLED_DISPLAY_NORMAL);
  char* str = (char*) malloc(20);
  for (uint8_t i = 1; i < MAX_PLAYERS_NUM; i++) {
    if (players[i].online) {
      sprintf(str, "%d", i + 1);
      oled_screen_display_text(str, (i - 1) * 32 + 8, 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_bitmap(figther_face_bmp, (i - 1) * 32 + 16, 8, 16, 8,
                                 OLED_DISPLAY_NORMAL);
    }
  }
  free(str);
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
      show_available_game();
      show_clients();
      break;
    case SHOW_UNCONNECTED:
      show_badge_unconnected();
      break;
    default:
      break;
  }
}

void update_bar(int16_t value, uint8_t bar_height, bool x_mirror) {
  uint8_t active_cols = (uint32_t) value * BAR_WIDTH / 10000;
  memset(bar_bitmap, 0, sizeof(bar_bitmap));
  for (int y = 0; y < bar_height; y++) {
    for (int x = 0; x < active_cols; x++) {
      if (x_mirror) {
        bar_bitmap[y][(BAR_WIDTH - 1 - x) / 8] |= (1 << (x % 8));
      } else {
        bar_bitmap[y][x / 8] |= (1 << (7 - (x % 8)));
      }
    }
  }
}

void rope_game_show_rope() {
  bool x_mirror = game_instance.rope_bar < 0;
  update_bar(abs(game_instance.rope_bar), 3, x_mirror);
  oled_screen_display_bitmap(bar_bitmap, x_mirror ? 0 : 64, 18, BAR_WIDTH,
                             BAR_HEIGHT, OLED_DISPLAY_NORMAL);
}

void rope_game_show_game_data() {
  oled_screen_clear();
  oled_screen_display_text("Team1", 0, 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text("Team2", 88, 0, OLED_DISPLAY_NORMAL);
  rope_game_show_rope();

  char* str = (char*) malloc(5);
  oled_screen_display_bitmap(figther_face_bmp, 8, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("1", 0, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", game_instance.players_data[0].strenght);
  oled_screen_display_text(
      str, 25, 1, my_id == 0 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 8, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("2", 0, 3, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", game_instance.players_data[1].strenght);
  oled_screen_display_text(
      str, 25, 3, my_id == 1 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 105, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("3", 120, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", game_instance.players_data[2].strenght);
  oled_screen_display_text(
      str, 80, 1, my_id == 2 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 105, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("4", 120, 3, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", game_instance.players_data[3].strenght);
  oled_screen_display_text(
      str, 80, 3, my_id == 3 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);
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

void games_screens_module_show_games_module_event(games_module_events_t event) {
  lobby_manager_set_display_status_cb(NULL);
  switch (event) {
    case NOT_ENOUGHT_BADGES_EVENT:
      break;
    default:
      break;
  }
  lobby_manager_set_display_status_cb(games_screens_module_show_lobby_state);
}
//////////////////////////////////////////////////////////////////////////////////
