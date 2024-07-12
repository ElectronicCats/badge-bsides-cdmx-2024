#include "games_screens_module.h"
#include <string.h>
#include "arm_wrestling_game.h"
#include "games_bitmaps.h"
#include "lobby_manager.h"
#include "oled_screen.h"
#include "rope_game.h"
#include "speed_bag_game.h"

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

bool show_game_text(uint8_t players_count) {
  bool waiting = false;
  static uint8_t frame = 0;
  frame = ++frame > 3 ? 0 : frame;
  oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);
  switch (players_count) {
    case 2:
      oled_screen_display_text("RAUL GAME", 4, 3, OLED_DISPLAY_NORMAL);
      break;
    case 3:
      oled_screen_display_text("Waiting 3/4", 4, 3, OLED_DISPLAY_NORMAL);
      show_scanning_dots(95, 3, 3);
      return true;
    case 4:
      oled_screen_display_text("ROPE GAME", 4, 3, OLED_DISPLAY_NORMAL);
      break;
    case 5:
      oled_screen_display_text("Waiting 5/5", 4, 3, OLED_DISPLAY_NORMAL);
      show_scanning_dots(95, 3, 3);
      return true;
    default:
      oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_bitmap(badge_connection_bmp_arr[frame / 2], 32, 8, 64,
                                 16, OLED_DISPLAY_NORMAL);
      oled_screen_display_text("Waiting 1/2", 4, 3, OLED_DISPLAY_NORMAL);
      show_scanning_dots(95, 3, 3);
      return true;
      break;
  }
  return false;
}
void show_client_state() {
  char* str = (char*) malloc(20);
  sprintf(str, " Client Mode ");
  oled_screen_clear_line(0, 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center(str, 0, OLED_DISPLAY_INVERT);
  sprintf(str, " YOU ARE-->P%d", my_client_id + 1);
  oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);
  oled_screen_clear_line(0, 2, OLED_DISPLAY_NORMAL);
  oled_screen_display_text(str, 0, 2, OLED_DISPLAY_NORMAL);
  oled_screen_display_bitmap(figther_face_bmp, 104, 16, 16, 8,
                             OLED_DISPLAY_NORMAL);
  free(str);
  show_game_text(get_clients_count());
}

void show_available_game() {
  uint8_t players_count = get_clients_count();
  if (!show_game_text(players_count))
    oled_screen_display_bitmap(enter_button_bmp, 112, 24, 16, 8,
                               OLED_DISPLAY_NORMAL);
}

void show_clients() {
  if (!get_clients_count()) {
    return;
  }
  oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);
  oled_screen_clear_line(0, 2, OLED_DISPLAY_NORMAL);
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
  static uint8_t print = 1;
  print = ++print > 4 ? 1 : print;
  if (print % 4)
    return;
  switch (state) {
    case HOST_STATE:
      show_host_state();
      break;
    case CLIENT_STATE:
      show_client_state();
      break;
    case SHOW_CLIENTS:
      oled_screen_clear_line(0, 0, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Connect Badges", 0, OLED_DISPLAY_NORMAL);
      show_clients();
      show_available_game();
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
  oled_screen_clear_line(0, 2, OLED_DISPLAY_NORMAL);
  uint8_t rope_idx = (abs(game_instance.rope_bar) / 80) % 4;
  oled_screen_display_bitmap(rope_bmp_arr[rope_idx], 0, 17, 128, 5,
                             OLED_DISPLAY_NORMAL);
  uint8_t badge_x_pos = ((game_instance.rope_bar + 10000) * 120) / (20000);
  oled_screen_display_bitmap(badge_bmp, badge_x_pos, 16, 8, 8,
                             OLED_DISPLAY_NORMAL);
}

void speed_game_show_bag() {
  // oled_screen_clear_line(0, 2, OLED_DISPLAY_NORMAL);
  // oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);
  // update_bar(speed_bag_game_instance.bag_bar, BAR_HEIGHT, false);
  if (speed_bag_game_instance.players_data[0].strenght == 0) {
    oled_screen_display_bitmap(speed_bag_frame_0, 0, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else if (speed_bag_game_instance.players_data[0].strenght % 2 == 0) {
    oled_screen_display_bitmap(speed_bag_frame_1, 0, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_bitmap(speed_bag_frame_1, 0, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  }
  if (speed_bag_game_instance.players_data[1].strenght % 2 == 0) {
    oled_screen_display_bitmap(speed_bag_frame_2, 24, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_bitmap(speed_bag_frame_1, 24, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  }
  if (speed_bag_game_instance.players_data[2].strenght % 2 == 0) {
    oled_screen_display_bitmap(speed_bag_frame_2, 48, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_bitmap(speed_bag_frame_1, 48, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  }
  if (speed_bag_game_instance.players_data[3].strenght % 2 == 0) {
    oled_screen_display_bitmap(speed_bag_frame_2, 72, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_bitmap(speed_bag_frame_1, 72, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  }
  if (speed_bag_game_instance.players_data[4].strenght % 2 == 0) {
    oled_screen_display_bitmap(speed_bag_frame_2, 96, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_bitmap(speed_bag_frame_1, 96, 16, 8, 8,
                               OLED_DISPLAY_NORMAL);
  }
  // oled_screen_display_bitmap(speed_bag_frame_0, 0, 16, 8, 8,
  //                            OLED_DISPLAY_NORMAL);
  // oled_screen_display_bitmap(speed_bag_frame_0, 24, 16, 8, 8,
  //                            OLED_DISPLAY_NORMAL);
  // oled_screen_display_bitmap(speed_bag_frame_0, 48, 16,  8, 8,
  //                            OLED_DISPLAY_NORMAL);
  // oled_screen_display_bitmap(speed_bag_frame_0, 72, 16, 8, 8,
  //                            OLED_DISPLAY_NORMAL);
  // oled_screen_display_bitmap(speed_bag_frame_0, 96, 16, 8, 8,
  //                            OLED_DISPLAY_NORMAL);
}

void arm_wrestling_show_game_data() {
  char* str = (char*) malloc(10);
  // oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 105, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("2", 120, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%s%03d", arm_wrestling_player_id == 1 ? "-->" : "",
          wgame_instance.players_data[1].strength);
  oled_screen_display_text(
      str, arm_wrestling_player_id == 1 ? 56 : 80, 1,
      arm_wrestling_player_id == 1 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 8, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("1", 0, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%03d%s", wgame_instance.players_data[0].strength,
          arm_wrestling_player_id == 0 ? "<--" : "");
  oled_screen_display_text(
      str, 25, 1,
      arm_wrestling_player_id == 0 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  free(str);
}

void rope_game_show_game_data() {
  char* str = (char*) malloc(10);
  // oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 105, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("3", 120, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%s%03d", rope_player_id == 2 ? "-->" : "",
          game_instance.players_data[2].strenght);
  oled_screen_display_text(
      str, rope_player_id == 2 ? 56 : 80, 1,
      rope_player_id == 2 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 8, 8, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("1", 0, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%03d%s", game_instance.players_data[0].strenght,
          rope_player_id == 0 ? "<--" : "");
  oled_screen_display_text(
      str, 25, 1,
      rope_player_id == 0 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  // oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 8, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_bitmap(figther_face_bmp, 105, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("4", 120, 3, OLED_DISPLAY_NORMAL);
  sprintf(str, "%s%03d", rope_player_id == 3 ? "-->" : "",
          game_instance.players_data[3].strenght);
  oled_screen_display_text(
      str, rope_player_id == 3 ? 56 : 80, 3,
      rope_player_id == 3 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_text("2", 0, 3, OLED_DISPLAY_NORMAL);
  sprintf(str, "%03d%s", game_instance.players_data[1].strenght,
          rope_player_id == 1 ? "<--" : "");
  oled_screen_display_text(
      str, 25, 3,
      rope_player_id == 1 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  oled_screen_display_bitmap(figther_face_bmp, 105, 24, 16, 8,
                             OLED_DISPLAY_NORMAL);
  oled_screen_display_text("4", 120, 3, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", game_instance.players_data[3].strenght);
  oled_screen_display_text(
      str, 80, 3,
      rope_player_id == 3 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);
  free(str);
}
void speed_bag_game_show_game_data() {
  char* str = (char*) malloc(5);

  // oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);
  //  TODO: FIx the current instanace, create the correct bitmap
  //  Player 1
  oled_screen_display_bitmap(figther_face_bmp, 0, 0, 16, 8,
                             OLED_DISPLAY_NORMAL);
  // oled_screen_display_text("1", 0, 1, OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", speed_bag_game_instance.players_data[0].strenght);
  oled_screen_display_text(
      str, 0, 3,
      speed_bag_player_id == 0 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  // Player 2
  oled_screen_display_bitmap(figther_face_bmp, 24, 0, 16, 8,
                             OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", speed_bag_game_instance.players_data[1].strenght);
  oled_screen_display_text(
      str, 24, 3,
      speed_bag_player_id == 1 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  // Player 3
  oled_screen_display_bitmap(figther_face_bmp, 48, 0, 16, 8,
                             OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", speed_bag_game_instance.players_data[2].strenght);
  oled_screen_display_text(
      str, 48, 3,
      speed_bag_player_id == 2 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  // Player 4
  oled_screen_display_bitmap(figther_face_bmp, 72, 0, 16, 8,
                             OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", speed_bag_game_instance.players_data[3].strenght);
  oled_screen_display_text(
      str, 72, 3,
      speed_bag_player_id == 3 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);

  // Player 5
  oled_screen_display_bitmap(figther_face_bmp, 96, 0, 16, 8,
                             OLED_DISPLAY_NORMAL);
  sprintf(str, "%d", speed_bag_game_instance.players_data[4].strenght);
  oled_screen_display_text(
      str, 96, 3,
      speed_bag_player_id == 4 ? OLED_DISPLAY_INVERT : OLED_DISPLAY_NORMAL);
  free(str);
}

void games_screen_module_show_game_over_arm(bool winner) {
  oled_screen_clear();
  char* str = (char*) malloc(16);
  if (winner == arm_wrestling_player_id) {
    sprintf(str, "   YOU WIN    ");
    oled_screen_display_text(str, 4, 0, OLED_DISPLAY_INVERT);
    oled_screen_display_bitmap(winner_belt, 28, 8, 64, 24, OLED_DISPLAY_NORMAL);
  } else {
    sprintf(str, "   YOU LOSE   ");
    oled_screen_display_text_center(str, 0, OLED_DISPLAY_NORMAL);
    sprintf(str, "   Try to be   ");
    oled_screen_display_text_center(str, 2, OLED_DISPLAY_NORMAL);
    sprintf(str, "    FASTER    ");
    oled_screen_display_text_center(str, 3, OLED_DISPLAY_NORMAL);
  }
  free(str);
}

void games_screens_module_show_game_over(bool winner) {
  oled_screen_clear();
  char* str = (char*) malloc(16);
  sprintf(str, "  TEAM %d WIN   ", winner + 1);
  oled_screen_display_text(str, 4, 0, OLED_DISPLAY_INVERT);
  if (winner && rope_player_id >= 2) {
    oled_screen_display_bitmap(winner_belt, 28, 8, 64, 24, OLED_DISPLAY_NORMAL);
  } else if (!winner && rope_player_id <= 1) {
    oled_screen_display_bitmap(winner_belt, 28, 8, 64, 24, OLED_DISPLAY_NORMAL);
  } else {
    sprintf(str, "   YOU LOSE   ");
    oled_screen_display_text_center(str, 1, OLED_DISPLAY_NORMAL);
    sprintf(str, "Try to be fast");
    oled_screen_display_text_center(str, 2, OLED_DISPLAY_NORMAL);
    sprintf(str, "& coordinated");
    oled_screen_display_text_center(str, 3, OLED_DISPLAY_NORMAL);
  }

  // printf("Team %d won\n", winner + 1);
  free(str);
}

void games_screen_module_show_game_over_speed(int winner) {
  oled_screen_clear();
  char* str = (char*) malloc(32);
  sprintf(str, "Player %d won\n", winner + 1);
  oled_screen_display_text_center(str, 0, OLED_DISPLAY_INVERT);
  printf("Player %d won\n", winner + 1);
  oled_screen_display_bitmap(game_belt, 0, 8, 128, 32, OLED_DISPLAY_NORMAL);
  free(str);
  // if (speed_bag_player_id != winner) {
  //   oled_screen_display_text_center("You lost", 2, OLED_DISPLAY_INVERT);
  // } else {
  //   char* str = (char*) malloc(32);
  //   sprintf(str, "Player %d won\n", winner + 1);
  //   oled_screen_display_text_center(str, 0, OLED_DISPLAY_INVERT);
  //   printf("Player %d won\n", winner + 1);
  //   oled_screen_display_bitmap(game_belt, 0, 8, 128, 32,
  //   OLED_DISPLAY_NORMAL); free(str);
  // }
}

void games_screens_module_show_rope_game_event(rope_game_events_t event) {
  switch (event) {
    case UPDATE_GAME_EVENT:
      oled_screen_display_text("Team1      Team2", 0, 0, OLED_DISPLAY_NORMAL);
      rope_game_show_rope();
      rope_game_show_game_data();
      break;
    default:
      break;
  }
}

void games_screens_module_show_arm_wrestling_game_event(
    arm_wrestling_events_t event) {
  switch (event) {
    case WUPDATE_GAME_EVENT:
      oled_screen_display_text("P1        P2", 0, 0, OLED_DISPLAY_NORMAL);
      // rope_game_show_rope();
      bool x_mirror = wgame_instance.arm_position < 0;
      update_bar(abs(wgame_instance.arm_position), 8, x_mirror);
      oled_screen_display_bitmap(bar_bitmap, x_mirror ? 0 : 64, 32, 64, 8,
                                 OLED_DISPLAY_NORMAL);
      arm_wrestling_show_game_data();
      break;
    default:
      break;
  }
}

void games_screens_module_show_speed_bag_game_event(
    speed_bag_speed_bag_game_events_t event) {
  switch (event) {
    case UPDATE_GAME_EVENT:
      speed_game_show_bag();
      // speed_bag_game_show_game_data();
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
