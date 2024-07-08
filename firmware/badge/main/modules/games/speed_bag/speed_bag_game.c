#include "speed_bag_game.h"
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
#include "oled_screen.h"
#include "stdbool.h"

#define NORMAL_TIMEOUT  200000
#define ANGRY_TIMEOUT   160000
#define FURIOUS_TIMEOUT 120000
#define HOST_MAC        game_players_mac[0]
#define MAX_SCORE       100

static const char* TAG = "Speed_Game";

// #define DESACTIVAR_PRINT

static uint8_t game_players_mac[MAX_BAG_GAME_PLAYERS][MAC_SIZE];
static bool host_mode;
static bool swap;
static bool is_game_running = false;
static esp_timer_handle_t timer_handle;
static TaskHandle_t speed_bag_game_task_handler = NULL;
static int speed_bag_winner = -1;
static void speed_bag_game_over();
static void speed_bag_game_exit();

uint8_t speed_bag_player_id;

static void game_data_init() {
  for (uint8_t i = 0; i < MAX_BAG_GAME_PLAYERS; i++) {
    memcpy(game_players_mac[i], players[i].mac, MAC_SIZE);
  }
  speed_bag_player_id = my_client_id;
  host_mode = !client_mode;

  memset(&speed_bag_game_instance, 0, sizeof(speed_bag_game_data_t));
  speed_bag_main_player =
      &speed_bag_game_instance.players_data[speed_bag_player_id];
}

static void speed_bag_print_game_data() {
  for (int8_t i = 0; i < MAX_BAG_GAME_PLAYERS; i++) {
    printf("P%d: %d%s\n", i + 1,
           speed_bag_game_instance.players_data[i].strenght,
           i == speed_bag_player_id ? "<-------" : "");
  }
  printf("Speed BAR: %d\n", speed_bag_game_instance.bag_bar);
}

static int8_t get_player_id(uint8_t* mac) {
  for (uint8_t i = 1; i < MAX_BAG_GAME_PLAYERS; i++) {
    if (memcmp(game_players_mac[i], mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}

// ///////////////////////////////////////////////////////////////////////////////

static void speed_bag_send_player_data() {
  speed_bag_player_data_cmd_t player_data_msg = {
      .cmd = SPEED_BAG_UPDATE_PLAYER_DATA_CMD,
      .player_data = *speed_bag_main_player};
  badge_connect_send(HOST_MAC, &player_data_msg,
                     sizeof(speed_bag_player_data_cmd_t));
}

static void speed_bag_send_game_data() {
  speed_bag_game_data_cmd_t game_data_msg = {
      .cmd = SPEED_BAG_UPDATE_GAME_DATA_CMD,
      .game_data = speed_bag_game_instance};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_data_msg,
                     sizeof(speed_bag_game_data_cmd_t));
}

static void send_update_data() {
  if (host_mode) {
    speed_bag_send_game_data();
  } else {
    speed_bag_send_player_data();
  }
}

// ///////////////////////////////////////////////////////////////////////////////

static void speed_bag_handle_player_update(badge_connect_recv_msg_t* msg) {
  uint8_t id = get_player_id(msg->src_addr);
  if (!host_mode || id < 1) {
    return;
  }
  speed_bag_player_data_cmd_t* data = (speed_bag_player_data_cmd_t*) msg->data;
  memcpy(&speed_bag_game_instance.players_data[id], &data->player_data,
         sizeof(speed_bag_player_data_t));
}

static void speed_bag_handle_game_update(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  speed_bag_game_data_cmd_t* game_data_msg =
      (speed_bag_game_data_cmd_t*) msg->data;
  for (uint8_t i = 0; i < MAX_BAG_GAME_PLAYERS; i++) {
    if (i == speed_bag_player_id)
      continue;
    memcpy(&speed_bag_game_instance.players_data[i],
           &game_data_msg->game_data.players_data[i],
           sizeof(speed_bag_player_data_t));
  }
  speed_bag_game_instance.bag_bar = game_data_msg->game_data.bag_bar;
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_stop_game_cmd() {
  speed_bag_stop_game_cmd_t cmd = {.cmd = SPEED_BAG_STOP_GAME_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd,
                     sizeof(speed_bag_stop_game_cmd_t));
}

static void speed_handle_stop_game_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0) {
    return;
  }
  ESP_LOGE(TAG, "STOP GAME speed_handle_stop_game_cmd");
  speed_bag_game_exit();
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_game_over_cmd() {
  ESP_LOGI(TAG, "send_game_over_cmd");
  speed_bag_game_over_cmd_t cmd = {.cmd = SPEED_BAG_GAME_OVER_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd,
                     sizeof(speed_bag_game_over_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(100));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd,
                     sizeof(speed_bag_game_over_cmd_t));
}

void speed_handle_game_over_cmd(badge_connect_recv_msg_t* msg) {
  ESP_LOGI(TAG, "GAME OVER speed_handle_game_over_cmd");
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0) {
    return;
  }
  speed_bag_game_over();
}

// ///////////////////////////////////////////////////////////////////////////////
static void speed_bag_game_over() {
  ESP_LOGI(TAG, "speed_bag_game_over");
  send_game_over_cmd();
  is_game_running = false;
  games_screen_module_show_game_over_speed(speed_bag_winner);
}

void update_speed_bag_value() {
  if (!host_mode) {
    return;
  }
  // speed_bag_game_instance.bag_bar +=
  //     speed_bag_game_instance.players_data[3].strenght +
  //     speed_bag_game_instance.players_data[2].strenght -
  //     speed_bag_game_instance.players_data[1].strenght -
  //     speed_bag_game_instance.players_data[0].strenght;

  ESP_LOGE(TAG, "update_speed_bag_value");

  if ((int) speed_bag_game_instance.players_data[0].strenght >= MAX_SCORE) {
    speed_bag_winner = 0;
    ESP_LOGE(TAG, "speed_bag_winner - players_data[0] %d", speed_bag_winner);
  }
  if ((int) speed_bag_game_instance.players_data[1].strenght >= MAX_SCORE) {
    speed_bag_winner = 1;
    ESP_LOGE(TAG, "speed_bag_winner - players_data[1] %d", speed_bag_winner);
  }
  if ((int) speed_bag_game_instance.players_data[2].strenght >= MAX_SCORE) {
    speed_bag_winner = 2;
    ESP_LOGE(TAG, "speed_bag_winner - players_data[2] %d", speed_bag_winner);
  }
  if ((int) speed_bag_game_instance.players_data[3].strenght >= MAX_SCORE) {
    speed_bag_winner = 3;
    ESP_LOGE(TAG, "speed_bag_winner - players_data[3] %d", speed_bag_winner);
  }
  if ((int) speed_bag_game_instance.players_data[4].strenght >= MAX_SCORE) {
    speed_bag_winner = 4;
    ESP_LOGE(TAG, "speed_bag_winner - players_data[4] %d", speed_bag_winner);
  }
  if (speed_bag_winner != -1) {
    speed_bag_game_over();
  }
}

static void speed_on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  ESP_LOGI(TAG, "speed_on_receive_data_cb");
  uint8_t cmd = *((uint8_t*) msg->data);
  ESP_LOGI(TAG, "CMD: %d", cmd);
  switch (cmd) {
    case SPEED_BAG_UPDATE_PLAYER_DATA_CMD:
      speed_bag_handle_player_update(msg);
      break;
    case SPEED_BAG_UPDATE_GAME_DATA_CMD:
      speed_bag_handle_game_update(msg);
      break;
    case SPEED_BAG_STOP_GAME_CMD:
      ESP_LOGI(TAG, "STOP GAME");
      speed_handle_stop_game_cmd(msg);
    case SPEED_BAG_GAME_OVER_CMD:
      ESP_LOGI(TAG, "GAME OVER");
      speed_handle_game_over_cmd(msg);
      break;
    default:
      break;
  }
}

// ///////////////////////////////////////////////////////////////////////////////

static void reset_timer() {
  esp_timer_stop(timer_handle);
  if (speed_bag_main_player->strenght != 0)
    esp_timer_start_once(
        timer_handle, speed_bag_main_player->strenght < SPEED_BAG_NORMAL_STATE
                          ? NORMAL_TIMEOUT
                      : speed_bag_main_player->strenght < SPEED_BAG_ANGRY_STATE
                          ? ANGRY_TIMEOUT
                          : FURIOUS_TIMEOUT);
}

static void increment_strenght() {
  reset_timer();
  speed_bag_main_player->strenght = speed_bag_main_player->strenght < 255
                                        ? speed_bag_main_player->strenght + 1
                                        : speed_bag_main_player->strenght;
}

static void decrement_strenght() {
  reset_timer();
  speed_bag_main_player->strenght = speed_bag_main_player->strenght < 10
                                        ? 0
                                        : speed_bag_main_player->strenght - 10;
}

// ///////////////////////////////////////////////////////////////////////////////

static void speed_bag_task() {
  oled_screen_clear();
  while (is_game_running) {
    // increment_strenght();
    games_screens_module_show_speed_bag_game_event(UPDATE_GAME_EVENT);
    send_update_data();
    update_speed_bag_value();
    speed_bag_print_game_data();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  speed_bag_game_task_handler = NULL;
  printf("speed_bag_task DELETED\n");
  vTaskDelete(NULL);
}
void speed_bag_game_init() {
  ESP_LOGE(TAG, "speed_bag_game_init");
  game_data_init();
  const esp_timer_create_args_t timer_args = {.callback = &decrement_strenght,
                                              .name = "example_timer"};

  esp_err_t ret = esp_timer_create(&timer_args, &timer_handle);
  if (ret != ESP_OK) {
    ESP_LOGE("Timer", "Failed to create timer: %s", esp_err_to_name(ret));
    return;
  }
  lobby_manager_register_custom_cmd_recv_cb(speed_on_receive_data_cb);
  is_game_running = true;
  menu_screens_set_app_state(true, speed_bag_game_input);
  xTaskCreate(speed_bag_task, "speed_bag_task", 4096, NULL, 15,
              &speed_bag_game_task_handler);
}
static void speed_bag_game_exit() {
  is_game_running = false;
  send_stop_game_cmd();
  lobby_manager_register_custom_cmd_recv_cb(NULL);
  if (timer_handle != NULL) {
    esp_timer_stop(timer_handle);
    esp_timer_delete(timer_handle);
    timer_handle = NULL;
  }
  send_stop_game_cmd();
  vTaskDelay(pdMS_TO_TICKS(50));
  send_stop_game_cmd();
  games_module_setup();
}

// ///////////////////////////////////////////////////////////////////////////////

void speed_bag_game_input(button_event_t button_pressed) {
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
  const char* button_name_str = button_names[button_name];
  const char* button_event_str = button_events_name[button_event];
  if (button_event != BUTTON_PRESS_DOWN)
    return;
  switch (button_name) {
    case BUTTON_LEFT:
      speed_bag_game_exit();
      break;
    case BUTTON_RIGHT:
      if (speed_bag_main_player->strenght == 0 || swap) {
        increment_strenght();
        swap = false;
      } else {
        decrement_strenght();
      }
      break;
    case BUTTON_UP:
      if (speed_bag_main_player->strenght == 0 || !swap) {
        increment_strenght();
        swap = true;
      } else {
        decrement_strenght();
      }
      break;
    default:
      break;
  }
}
