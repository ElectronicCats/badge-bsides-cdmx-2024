#include "games_module.h"
#include "arm_wrestling_game.h"
#include "badge_connect.h"
#include "esp_log.h"
#include "espnow.h"
#include "games_screens_module.h"
#include "lobby_manager.h"
#include "menu_screens_modules.h"
#include "oled_screen.h"
#include "rope_game.h"
#include "speed_bag_game.h"

void handle_games_module_cmds(badge_connect_recv_msg_t* msg);

void games_module_app_selector();
void games_module_state_machine(button_event_t button_pressed);

void games_module_begin() {
  games_module_setup();
  lobby_manager_init();
}
void games_module_setup() {
  vTaskDelay(pdMS_TO_TICKS(100));
  menu_screens_set_app_state(true, games_module_state_machine);
  oled_screen_clear(OLED_DISPLAY_NORMAL);
  lobby_manager_set_display_status_cb(games_screens_module_show_lobby_state);
  lobby_manager_register_custom_cmd_recv_cb(handle_games_module_cmds);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void open_game(uint8_t game_id) {
  lobby_manager_set_display_status_cb(NULL);
  lobby_manager_register_custom_cmd_recv_cb(NULL);
  switch (game_id) {
    case RAUL_GAME:
      arm_wrestling_init();
      break;
    case ROPE_GAME:
      rope_game_init();
      break;
    case KEVIN_GAME:
      oled_screen_clear(OLED_DISPLAY_NORMAL);
      speed_bag_game_init();
      break;
    default:
      break;
  }
}
void send_start_game_cmd() {
  if (client_mode)
    return;
  uint8_t players_count = get_clients_count();
  uint8_t game_id = 0;
  switch (players_count) {
    case 2:
      game_id = RAUL_GAME;
      break;
    case 4:
      game_id = ROPE_GAME;
      break;
    case 5:
      game_id = KEVIN_GAME;
      break;
    default:
      return;
      break;
  }
  start_game_cmd_t cmd = {.cmd = START_GAME, .game_id = game_id};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(start_game_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(50));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(start_game_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(50));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(start_game_cmd_t));
  open_game(game_id);
}

void handle_start_game_cmd(badge_connect_recv_msg_t* msg) {
  if (memcmp(host_mac, msg->src_addr, MAC_SIZE) != 0)
    return;
  start_game_cmd_t* cmd = (start_game_cmd_t*) msg->data;
  open_game(cmd->game_id);
}

void handle_games_module_cmds(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  printf("GAMES_MODULE -> CMD: %d\n", cmd);
  switch (cmd) {
    case START_GAME:
      handle_start_game_cmd(msg);
      break;
    default:
      break;
  }
}

void games_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_GAMES_MODULE, "Games engine state machine from team: %d %d",
           button_name, button_event);
  switch (button_name) {
    case BUTTON_RIGHT:
      switch (button_event) {
        case BUTTON_PRESS_DOWN:
          send_start_game_cmd();
          break;
        case BUTTON_PRESS_UP:
          break;
      }
      break;
    case BUTTON_LEFT:
      switch (button_event) {
        case BUTTON_PRESS_DOWN:
          printf("GAMES DEINIT\n");
          screen_module_set_screen(MENU_GAMES_PLAY);
          esp_restart();
          // lobby_manager_deinit();
          // menu_screens_set_app_state(false, NULL);
          // menu_screens_exit_submenu();
          break;
      }
      break;
    default:
      break;
  }
}
