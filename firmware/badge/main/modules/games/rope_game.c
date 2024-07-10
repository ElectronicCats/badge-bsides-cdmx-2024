#include "rope_game.h"
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

#ifdef DESACTIVAR_PRINT
  #define printf(fmt, ...) ((void) 0)
#endif

static const char* TAG = "Rope_Game_Client";

// #define DESACTIVAR_PRINT

static uint8_t game_players_mac[MAX_ROPE_GAME_PLAYERS][MAC_SIZE];
static bool host_mode;
static bool swap;
static bool is_game_running = false;
static esp_timer_handle_t timer_handle;
static TaskHandle_t rope_game_task_handler = NULL;
static void rope_game_over();
static void rope_game_exit();
// static void rope_send_player_data(void);
// static void rope_print_game_data(void);
// static int8_t get_player_id(uint8_t* mac);
// static void rope_send_game_data();
// static void send_update_data();
// static void rope_handle_player_update(badge_connect_recv_msg_t* msg);
// static void rope_handle_game_update(badge_connect_recv_msg_t* msg);
// static void send_stop_game_cmd();
// static void handle_stop_game_cmd(badge_connect_recv_msg_t* msg);
// static void send_game_over_cmd();
// static void handle_game_over_cmd(badge_connect_recv_msg_t* msg);

uint8_t rope_player_id;

// void rope_game_input(button_event_t button_pressed);
//  void rope_game_exit();
//  void rope_game_over();

static void game_data_init() {
  for (uint8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    memcpy(game_players_mac[i], players[i].mac, MAC_SIZE);
  }
  rope_player_id = my_client_id;
  host_mode = !client_mode;

  memset(&game_instance, 0, sizeof(game_data_t));
  me = &game_instance.players_data[rope_player_id];
}

static void rope_print_game_data() {
  for (int8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    printf("P%d: %d%s\n", i + 1, game_instance.players_data[i].strenght,
           i == rope_player_id ? "<-------" : "");
  }
  printf("ROPE BAR: %d\n", game_instance.rope_bar);
}

static int8_t get_player_id(uint8_t* mac) {
  for (uint8_t i = 1; i < MAX_ROPE_GAME_PLAYERS; i++) {
    if (memcmp(game_players_mac[i], mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}

// ///////////////////////////////////////////////////////////////////////////////

static void rope_send_player_data() {
  player_data_cmd_t player_data_msg = {.cmd = UPDATE_PLAYER_DATA_CMD,
                                       .player_data = *me};
  badge_connect_send(HOST_MAC, &player_data_msg, sizeof(player_data_cmd_t));
}

static void rope_send_game_data() {
  game_data_cmd_t game_data_msg = {.cmd = UPDATE_GAME_DATA_CMD,
                                   .game_data = game_instance};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_data_msg,
                     sizeof(game_data_cmd_t));
}

static void send_update_data() {
  if (host_mode)
    rope_send_game_data();
  else
    rope_send_player_data();
}

// ///////////////////////////////////////////////////////////////////////////////

static void rope_handle_player_update(badge_connect_recv_msg_t* msg) {
  uint8_t id = get_player_id(msg->src_addr);
  if (!host_mode || id < 1) {
    return;
  }
  player_data_cmd_t* data = (player_data_cmd_t*) msg->data;
  memcpy(&game_instance.players_data[id], &data->player_data,
         sizeof(player_data_t));
}

static void rope_handle_game_update(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  game_data_cmd_t* game_data_msg = (game_data_cmd_t*) msg->data;
  for (uint8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    if (i == rope_player_id)
      continue;
    memcpy(&game_instance.players_data[i],
           &game_data_msg->game_data.players_data[i], sizeof(player_data_t));
  }
  game_instance.rope_bar = game_data_msg->game_data.rope_bar;
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_stop_game_cmd() {
  stop_game_cmd_t cmd = {.cmd = STOP_ROPE_GAME_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(stop_game_cmd_t));
}

static void handle_stop_game_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  rope_game_exit();
}

// ///////////////////////////////////////////////////////////////////////////////

static void send_game_over_cmd() {
  game_over_cmd_t cmd = {.cmd = GAME_OVER_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(game_over_cmd_t));
  vTaskDelay(pdMS_TO_TICKS(100));
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(game_over_cmd_t));
}

void handle_game_over_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  rope_game_over();
}

// ///////////////////////////////////////////////////////////////////////////////
static void rope_game_over() {
  send_game_over_cmd();
  is_game_running = false;
  games_screens_module_show_game_over(game_instance.rope_bar > 0);
}

void update_rope_bar_value() {
  if (!host_mode)
    return;
  game_instance.rope_bar += game_instance.players_data[3].strenght +
                            game_instance.players_data[2].strenght -
                            game_instance.players_data[1].strenght -
                            game_instance.players_data[0].strenght;
  if (abs(game_instance.rope_bar) > 10000) {
    rope_game_over();
  }
}

static void on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  switch (cmd) {
    case UPDATE_PLAYER_DATA_CMD:
      rope_handle_player_update(msg);
      break;
    case UPDATE_GAME_DATA_CMD:
      rope_handle_game_update(msg);
      break;
    case STOP_ROPE_GAME_CMD:
      handle_stop_game_cmd(msg);
    case GAME_OVER_CMD:
      handle_game_over_cmd(msg);
      break;
    default:
      break;
  }
}

// ///////////////////////////////////////////////////////////////////////////////

static void reset_timer() {
  esp_timer_stop(timer_handle);
  if (me->strenght != 0)
    esp_timer_start_once(timer_handle,
                         me->strenght < NORMAL_STATE  ? NORMAL_TIMEOUT
                         : me->strenght < ANGRY_STATE ? ANGRY_TIMEOUT
                                                      : FURIOUS_TIMEOUT);
}

static void increment_strenght() {
  reset_timer();
  me->strenght = me->strenght < 255 ? me->strenght + 1 : me->strenght;
}

static void decrement_strenght() {
  reset_timer();
  me->strenght = me->strenght < 10 ? 0 : me->strenght - 10;
}

// ///////////////////////////////////////////////////////////////////////////////

static void rope_game_task() {
  while (is_game_running) {
    // increment_strenght();
    games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
    send_update_data();
    update_rope_bar_value();
    rope_print_game_data();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  rope_game_task_handler = NULL;
  printf("rope_game_task DELETED\n");
  vTaskDelete(NULL);
}
void rope_game_init() {
  oled_screen_clear();
  game_data_init();
  oled_screen_clear();
  oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);
  const esp_timer_create_args_t timer_args = {.callback = &decrement_strenght,
                                              .name = "example_timer"};

  esp_err_t ret = esp_timer_create(&timer_args, &timer_handle);
  if (ret != ESP_OK) {
    ESP_LOGE("Timer", "Failed to create timer: %s", esp_err_to_name(ret));
    return;
  }
  lobby_manager_register_custom_cmd_recv_cb(on_receive_data_cb);
  is_game_running = true;
  menu_screens_set_app_state(true, rope_game_input);
  xTaskCreate(rope_game_task, "rope_game_task", 4096, NULL, 15,
              &rope_game_task_handler);
}
static void rope_game_exit() {
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

void rope_game_input(button_event_t button_pressed) {
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
  const char* button_name_str = button_names[button_name];
  const char* button_event_str = button_events_name[button_event];
  if (button_event != BUTTON_PRESS_DOWN)
    return;
  switch (button_name) {
    case BUTTON_LEFT:
      rope_game_exit();
      break;
    case BUTTON_RIGHT:
      if (me->strenght == 0 || swap) {
        increment_strenght();
        swap = false;
      } else {
        decrement_strenght();
      }
      break;
    case BUTTON_UP:
      if (me->strenght == 0 || !swap) {
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
#undef DESACTIVAR_PRINT
