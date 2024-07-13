#include "arm_wrestling_game.h"
#include <string.h>
#include "badge_connect.h"
#include "esp_log.h"
#include "espnow.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "games_module.h"
#include "games_screens_module.h"
#include "lobby_manager.h"
#include "menu_screens_modules.h"
#include "neopixels_module.h"
#include "oled_screen.h"
#include "stdbool.h"

#define HOST_MAC game_players_mac[0]
#define TEAM1    0
#define TEAM2    1

#ifdef DESACTIVAR_PRINT
  #define printf(fmt, ...) ((void) 0)
#endif

static const char* TAG = "Arm_Wrestling_Game";

static uint8_t game_players_mac[MAX_ARM_WRESTLING_PLAYERS][MAC_SIZE];
static bool host_mode;
static bool swap;
static bool team;
static bool winner;
static bool is_game_running = false;
static esp_timer_handle_t timer_handle;
static TaskHandle_t arm_wrestling_task_handler = NULL;

uint8_t arm_wrestling_player_id;

static void arm_wrestling_game_over();
static void arm_wrestling_exit();

static void set_team_color(bool team) {
  if (team == TEAM2) {
    neopixels_set_pixels(MAX_LED_NUMBER, 50, 50, 0);  // YELLOW
  } else {
    neopixels_set_pixels(MAX_LED_NUMBER, 0, 0, 50);  // BLUE
  }
  neopixels_refresh();
}

static void game_data_init() {
  for (uint8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    memcpy(game_players_mac[i], players[i].mac, MAC_SIZE);
  }
  arm_wrestling_player_id = my_client_id;
  host_mode = !client_mode;

  memset(&wgame_instance, 0, sizeof(wgame_data_t));
  wme = &wgame_instance.players_data[arm_wrestling_player_id];

  team = arm_wrestling_player_id;
  set_team_color(team);
}

static void arm_wrestling_print_game_data() {
  for (int8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    printf("P%d: %d%s\n", i + 1, wgame_instance.players_data[i].strength,
           i == arm_wrestling_player_id ? "<-------" : "");
  }
  printf("ARM POSITION: %d\n", wgame_instance.arm_position);
}

static int8_t get_player_id(uint8_t* mac) {
  for (uint8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    if (memcmp(game_players_mac[i], mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}

// ///////////////////////////////////////////////////////////////////////////////

static void arm_wrestling_send_player_data() {
  wplayer_data_cmd_t player_data_msg = {.cmd = WUPDATE_PLAYER_DATA_CMD,
                                        .player_data = *wme};
  badge_connect_send(HOST_MAC, &player_data_msg, sizeof(wplayer_data_cmd_t));
}

static void arm_wrestling_send_game_data() {
  wgame_data_cmd_t game_data_msg = {.cmd = WUPDATE_GAME_DATA_CMD,
                                    .game_data = wgame_instance};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_data_msg,
                     sizeof(wgame_data_cmd_t));
}

static void send_update_data() {
  if (host_mode)
    arm_wrestling_send_game_data();
  else
    arm_wrestling_send_player_data();
}

// ///////////////////////////////////////////////////////////////////////////////

static void arm_wrestling_handle_player_update(badge_connect_recv_msg_t* msg) {
  uint8_t id = get_player_id(msg->src_addr);
  if (!host_mode || id < 1 || id >= MAX_ARM_WRESTLING_PLAYERS) {
    return;
  }
  wplayer_data_cmd_t* data = (wplayer_data_cmd_t*) msg->data;
  memcpy(&wgame_instance.players_data[id], &data->player_data,
         sizeof(wplayer_data_t));
}

static void arm_wrestling_handle_game_update(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  wgame_data_cmd_t* game_data_msg = (wgame_data_cmd_t*) msg->data;
  memcpy(&wgame_instance, &game_data_msg->game_data, sizeof(wgame_data_t));
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_stop_game_cmd() {
  wstop_game_cmd_t cmd = {.cmd = WSTOP_ARM_WRESTLING_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(wstop_game_cmd_t));
}

static void handle_stop_game_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  arm_wrestling_exit();
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_game_over_cmd() {
  wgame_over_cmd_t cmd = {.cmd = WGAME_OVER_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(wgame_over_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(100));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(wgame_over_cmd_t));
}

static void handle_game_over_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  arm_wrestling_game_over();
}

// ///////////////////////////////////////////////////////////////////////////////

static void arm_wrestling_game_over() {
  send_game_over_cmd();
  is_game_running = false;
  winner = wgame_instance.arm_position > 0;
  games_screen_module_show_game_over_arm(winner);
  if (team == winner) {
    neopixels_set_pixels(MAX_LED_NUMBER, 0, 50, 0);  // GREEN
  } else {
    neopixels_set_pixels(MAX_LED_NUMBER, 50, 0, 0);  // RED
  }
  neopixels_refresh();
}

static void update_arm_position() {
  if (!host_mode)
    return;
  wgame_instance.arm_position += wgame_instance.players_data[1].strength -
                                 wgame_instance.players_data[0].strength;
  if (abs(wgame_instance.arm_position) > 10000) {
    arm_wrestling_game_over();
  }
}

static void on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  ESP_LOGI(TAG, "RECIVED DATA FROM OTHER GAME");
  uint8_t cmd = *((uint8_t*) msg->data);
  switch (cmd) {
    case WUPDATE_PLAYER_DATA_CMD:
      arm_wrestling_handle_player_update(msg);
      break;
    case WUPDATE_GAME_DATA_CMD:
      arm_wrestling_handle_game_update(msg);
      break;
    case WSTOP_ARM_WRESTLING_CMD:
      handle_stop_game_cmd(msg);
      break;
    case WGAME_OVER_CMD:
      handle_game_over_cmd(msg);
      break;
    default:
      break;
  }
}

// ///////////////////////////////////////////////////////////////////////////////

static void arm_wrestling_task() {
  oled_screen_clear();
  while (is_game_running) {
    games_screens_module_show_arm_wrestling_game_event(WUPDATE_GAME_EVENT);
    send_update_data();
    update_arm_position();
    arm_wrestling_print_game_data();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  arm_wrestling_task_handler = NULL;
  printf("arm_wrestling_task DELETED");
  vTaskDelete(NULL);
}

void arm_wrestling_init() {
  game_data_init();
  oled_screen_clear();
  oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);

  lobby_manager_register_custom_cmd_recv_cb(on_receive_data_cb);
  is_game_running = true;
  menu_screens_set_app_state(true, arm_wrestling_input);
  xTaskCreate(arm_wrestling_task, "arm_wrestling_task", 4096, NULL, 15,
              &arm_wrestling_task_handler);
}

static void arm_wrestling_exit() {
  is_game_running = false;
  send_stop_game_cmd();
  lobby_manager_register_custom_cmd_recv_cb(NULL);
  send_stop_game_cmd();
  vTaskDelay(pdMS_TO_TICKS(50));
  send_stop_game_cmd();
  neopixels_set_pixels(MAX_LED_NUMBER, 0, 0, 0);  // OFF
  neopixels_refresh();
  games_module_setup();
}

void arm_wrestling_input(button_event_t button_pressed) {
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
  if (button_event != BUTTON_PRESS_DOWN)
    return;
  switch (button_name) {
    case BUTTON_LEFT:
      arm_wrestling_exit();
      break;
    case BUTTON_RIGHT:
    case BUTTON_UP:
      wme->strength = wme->strength < 255 ? wme->strength + 1 : wme->strength;
      break;
    default:
      break;
  }
}
