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
#include "stdbool.h"

static const char* TAG = "Rope_Game_Client";

#define NORMAL_TIMEOUT  200000
#define ANGRY_TIMEOUT   160000
#define FURIOUS_TIMEOUT 120000

#define HOST_MAC game_players_mac[0]

// #define DESACTIVAR_PRINT

#ifdef DESACTIVAR_PRINT
  #define printf(fmt, ...) ((void) 0)
#endif

uint8_t game_players_mac[MAX_ROPE_GAME_PLAYERS][MAC_SIZE];
bool host_mode;
uint8_t my_id;

bool swap;
esp_timer_handle_t timer_handle;
TaskHandle_t rope_game_task_handler = NULL;

void rope_game_input(button_event_t button_pressed);
void rope_game_exit();

void game_data_init() {
  for (uint8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    memcpy(game_players_mac[i], players[i].mac, MAC_SIZE);
  }
  my_id = my_client_id;
  host_mode = !client_mode;

  memset(&game_instance, 0, sizeof(game_data_t));
  me = &game_instance.players_data[my_id];
}

void print_game_data() {
  for (int8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    printf("P%d: %d%s\n", i + 1, game_instance.players_data[i].strenght,
           i == my_id ? "<-------" : "");
  }
}

void print_mac(uint8_t mac[MAC_SIZE]) {
  for (int i = 0; i < MAC_SIZE; i++) {
    printf("%02X", mac[i]);
    if (i < MAC_SIZE - 1) {
      printf(":");
    }
  }
  printf("\n");
}

int8_t get_player_id(uint8_t* mac) {
  for (uint8_t i = 1; i < MAX_ROPE_GAME_PLAYERS; i++) {
    if (memcmp(players[i].mac, mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}

// ///////////////////////////////////////////////////////////////////////////////

void send_player_data() {
  player_data_cmd_t player_data_msg = {.cmd = UPDATE_PLAYER_DATA_CMD,
                                       .player_data = *me};
  badge_connect_send(HOST_MAC, &player_data_msg, sizeof(player_data_cmd_t));
}

void send_game_data() {
  game_data_cmd_t game_data_msg = {.cmd = UPDATE_GAME_DATA_CMD,
                                   .game_data = game_instance};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_data_msg,
                     sizeof(game_data_t));
}

void send_update_data() {
  if (host_mode)
    send_game_data();
  else
    send_player_data();
}

// ///////////////////////////////////////////////////////////////////////////////

void handle_player_update(badge_connect_recv_msg_t* msg) {
  uint8_t id = get_player_id(msg->src_addr);
  if (!host_mode || id < 1) {
    return;
  }
  player_data_cmd_t* data = (player_data_cmd_t*) msg->data;
  memcpy(&game_instance.players_data[id], &data->player_data,
         sizeof(player_data_t));
}

void handle_game_update(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  game_data_cmd_t* game_data_msg = (game_data_cmd_t*) msg->data;
  for (uint8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    if (i == my_id)
      continue;
    memcpy(&game_instance.players_data[i],
           &game_data_msg->game_data.players_data[i], sizeof(player_data_t));
  }
}

// ///////////////////////////////////////////////////////////////////////////////

void send_stop_game_cmd() {
  stop_game_cmd_t cmd = {.cmd = STOP_ROPE_GAME_CMD};
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &cmd, sizeof(stop_game_cmd_t));
}

void handle_stop_game_cmd(badge_connect_recv_msg_t* msg) {
  if (host_mode || memcmp(HOST_MAC, msg->src_addr, MAC_SIZE) != 0)
    return;
  rope_game_exit();
}

// ///////////////////////////////////////////////////////////////////////////////

void on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  // printf("ROPE GAME -> CMD: %d\n", cmd);
  switch (cmd) {
    case UPDATE_PLAYER_DATA_CMD:
      handle_player_update(msg);
      break;
    case UPDATE_GAME_DATA_CMD:
      handle_game_update(msg);
      break;
    case STOP_ROPE_GAME_CMD:
      handle_stop_game_cmd(msg);
      break;
    default:
      break;
  }
}

// ///////////////////////////////////////////////////////////////////////////////

void reset_timer() {
  esp_timer_stop(timer_handle);
  if (me->strenght != 0)
    esp_timer_start_once(timer_handle,
                         me->strenght < NORMAL_STATE  ? NORMAL_TIMEOUT
                         : me->strenght < ANGRY_STATE ? ANGRY_TIMEOUT
                                                      : FURIOUS_TIMEOUT);
}

void increment_strenght() {
  reset_timer();
  me->strenght = me->strenght < 255 ? me->strenght + 1 : me->strenght;
}

void decrement_strenght() {
  reset_timer();
  me->strenght = me->strenght < 10 ? 0 : me->strenght - 10;
}

// ///////////////////////////////////////////////////////////////////////////////

void rope_game_task() {
  while (1) {
    // increment_strenght();
    games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
    send_update_data();
    print_game_data();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void rope_game_init() {
  game_data_init();
  lobby_manager_set_display_status_cb(NULL);
  lobby_manager_register_custom_cmd_recv_cb(on_receive_data_cb);
  const esp_timer_create_args_t timer_args = {.callback = &decrement_strenght,
                                              .name = "example_timer"};

  esp_err_t ret = esp_timer_create(&timer_args, &timer_handle);
  if (ret != ESP_OK) {
    ESP_LOGE("Timer", "Failed to create timer: %s", esp_err_to_name(ret));
    return;
  }
  menu_screens_set_app_state(true, rope_game_input);
  xTaskCreate(rope_game_task, "rope_game_task", 2048, NULL, 10,
              &rope_game_task_handler);
}
void rope_game_exit() {
  send_stop_game_cmd();
  vTaskDelete(rope_game_task_handler);
  lobby_manager_register_custom_cmd_recv_cb(NULL);
  esp_err_t ret = esp_timer_delete(timer_handle);
  if (ret != ESP_OK) {
    ESP_LOGE("Timer", "Failed to delete timer: %s", esp_err_to_name(ret));
  }
  vTaskDelay(pdMS_TO_TICKS(100));
  games_module_setup();
}

// ///////////////////////////////////////////////////////////////////////////////

void rope_game_input(button_event_t button_pressed) {
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
  const char* button_name_str = button_names[button_name];
  const char* button_event_str = button_events_name[button_event];
  switch (button_name) {
    case BUTTON_LEFT:
      switch (button_event) {
        case BUTTON_PRESS_DOWN:
          rope_game_exit();
          break;
      }
    case BUTTON_RIGHT:
      if (button_event == BUTTON_PRESS_DOWN) {
        if (me->strenght == 0 || swap) {
          increment_strenght();
          swap = false;
        } else {
          decrement_strenght();
        }
      }
      break;
    case BUTTON_UP:
      if (button_event == BUTTON_PRESS_DOWN) {
        if (me->strenght == 0 || !swap) {
          increment_strenght();
          swap = true;
        } else {
          decrement_strenght();
        }
      }
      break;
    case BUTTON_DOWN:
      if (button_event == BUTTON_PRESS_DOWN) {
        printf("DOWN button pressed down\n");
      }
      break;
    default:
      printf("Unknown button pressed\n");
      break;
  }
}
#undef DESACTIVAR_PRINT
