#include <string.h>
#include "arm_wrestling.h"
#include "badge_connect.h"
#include "esp_log.h"
#include "espnow.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "games_module.h"
#include "games_screens_module.h"
#include "lobby_manager.h"
#include "menu_screens_modules.h"
#include "oled_screen.h"
#include "stdbool.h"

#define HOST_MAC game_players_mac[0]

#ifdef DESACTIVAR_PRINT
  #define printf(fmt, ...) ((void) 0)
#endif

static const char* TAG = "Arm_Wrestling_Game";

static uint8_t game_players_mac[MAX_ARM_WRESTLING_PLAYERS][MAC_SIZE];
static bool host_mode;
static bool swap;
static bool is_game_running = false;
// static esp_timer_handle_t timer_handle;
static TaskHandle_t arm_wrestling_task_handler = NULL;

uint8_t arm_wrestling_player_id;

static void arm_wrestling_game_over();
static void arm_wrestling_exit();

static void game_data_init() {
  for (uint8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    memcpy(game_players_mac[i], players[i].mac, MAC_SIZE);
  }
  arm_wrestling_player_id = my_client_id;
  host_mode = !client_mode;

  memset(&game_instance, 0, sizeof(game_data_t));
  me = &game_instance.players_data[arm_wrestling_player_id];
}

static void arm_wrestling_print_game_data() {
  for (int8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    ESP_LOGI(TAG, "P%d: %d%s", i + 1, game_instance.players_data[i].strength,
             i == arm_wrestling_player_id ? "<-------" : "");
  }
  ESP_LOGI(TAG, "ARM POSITION: %d", game_instance.arm_position);
}

static int8_t get_player_id(uint8_t* mac) {
  for (uint8_t i = 0; i < MAX_ARM_WRESTLING_PLAYERS; i++) {
    if (memcmp(game_players_mac[i], mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}

static void arm_wrestling_send_player_data() {
  player_data_cmd_t player_data_msg = {.cmd = UPDATE_PLAYER_DATA_CMD,
                                       .player_data = *me};
  badge_connect_send(HOST_MAC, &player_data_msg, sizeof(player_data_cmd_t));
}

static void arm_wrestling_send_game_data() {
  game_data_cmd_t game_data_msg = {.cmd = UPDATE_GAME_DATA_CMD,
                                   .game_data = game_instance};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_data_msg,
                     sizeof(game_data_cmd_t));
}

static void send_update_data() {
  if (host_mode)
    arm_wrestling_send_game_data();
  else
    arm_wrestling_send_player_data();
}

static void arm_wrestling_handle_player_update(badge_connect_recv_msg_t* msg) {
  uint8_t id = get_player_id(msg->src_addr);
  if (!host_mode || id < 0 || id >= MAX_ARM_WRESTLING_PLAYERS) {
    return;
  }
  player_data_cmd_t* data = (player_data_cmd_t*) msg->data;
  memcpy(&game_instance.players_data[id], &data->player_data,
         sizeof(player_data_t));
}

static void arm_wrestling_handle_game_update(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  game_data_cmd_t* game_data_msg = (game_data_cmd_t*) msg->data;
  memcpy(&game_instance, &game_data_msg->game_data, sizeof(game_data_t));
}

static void send_stop_game_cmd() {
  stop_game_cmd_t cmd = {.cmd = STOP_ARM_WRESTLING_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(stop_game_cmd_t));
}

static void handle_stop_game_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  arm_wrestling_exit();
}

static void send_game_over_cmd() {
  game_over_cmd_t cmd = {.cmd = GAME_OVER_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(game_over_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(100));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(game_over_cmd_t));
}

static void handle_game_over_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  arm_wrestling_game_over();
}

static void arm_wrestling_game_over() {
  send_game_over_cmd();
  is_game_running = false;
  games_screens_module_show_game_over(game_instance.arm_position > 0);
}

static void update_arm_position() {
  if (!host_mode)
    return;
  game_instance.arm_position += game_instance.players_data[1].strength -
                                game_instance.players_data[0].strength;
  if (abs(game_instance.arm_position) > 10000) {
    arm_wrestling_game_over();
  }
}

static void on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  switch (cmd) {
    case UPDATE_PLAYER_DATA_CMD:
      arm_wrestling_handle_player_update(msg);
      break;
    case UPDATE_GAME_DATA_CMD:
      arm_wrestling_handle_game_update(msg);
      break;
    case STOP_ARM_WRESTLING_CMD:
      handle_stop_game_cmd(msg);
      break;
    case GAME_OVER_CMD:
      handle_game_over_cmd(msg);
      break;
    default:
      break;
  }
}

static void arm_wrestling_task() {
  oled_screen_clear();
  while (is_game_running) {
    games_screens_module_show_arm_wrestling_event(UPDATE_GAME_EVENT);
    send_update_data();
    update_arm_position();
    arm_wrestling_print_game_data();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  arm_wrestling_task_handler = NULL;
  ESP_LOGI(TAG, "arm_wrestling_task DELETED");
  vTaskDelete(NULL);
}

void arm_wrestling_init() {
  game_data_init();
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
      me->strength = me->strength < 255 ? me->strength + 1 : me->strength;
      break;
    default:
      break;
  }
}
