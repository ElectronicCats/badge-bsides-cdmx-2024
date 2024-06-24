#include "rope_game.h"
#include <string.h>
#include "badge_connect.h"
#include "esp_log.h"
#include "espnow.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "games_screens_module.h"
#include "lobby_manager.h"
#include "menu_screens_modules.h"
#include "stdbool.h"

static const char* TAG = "Rope_Game_Client";

#define NORMAL_TIMEOUT  200000
#define ANGRY_TIMEOUT   160000
#define FURIOUS_TIMEOUT 120000

bool swap;
esp_timer_handle_t timer_handle;

void rope_game_input(button_event_t button_pressed);

void clear_game_data() {
  memset(&game_instance, 0, sizeof(game_data_t));
  game_instance.cmd = UPDATE_GAME_DATA;
}

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
  games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
  printf("%d\n", me->strenght);
}

void decrement_strenght() {
  reset_timer();
  me->strenght = me->strenght < 10 ? 0 : me->strenght - 10;
  games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
  printf("%d\n", me->strenght);
}

void player_init() {
  me = &game_instance.players_data[my_player_id];
  me->cmd = UPDATE_PLAYER_DATA;
  me->ID = my_player_id;
  me->strenght = 0;
}

void update_player_data() {
  me->ID = my_player_id;
  badge_connect_send(host_mac, me, sizeof(player_data_t));
}

void update_game_data() {
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &game_instance,
                     sizeof(game_data_t));
}

void handle_player_update(player_data_t* player_data) {
  printf("ROPE GAME -> handle_player_update\n");
  printf("ROPE GAME -> ID: %d\tStrenght: %d\n", player_data->ID,
         player_data->strenght);
  memcpy(&game_instance.players_data[player_data->ID], player_data,
         sizeof(player_data_t));
  games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
}

void handle_game_update(game_data_t* game_data) {
  printf("ROPE GAME -> handle_player_update\n");
  for (uint8_t i = 0; i < MAX_ROPE_GAME_PLAYERS; i++) {
    if (i == my_player_id)
      continue;
    memcpy(&game_instance.players_data[i], &game_data->players_data[i],
           sizeof(player_data_t));
  }
  games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
}

void on_receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  printf("ROPE GAME -> CMD: %d\n", cmd);
  switch (cmd) {
    case UPDATE_PLAYER_DATA:
      handle_player_update(msg->data);
      break;
    case UPDATE_GAME_DATA:
      if (memcmp(host_mac, msg->src_addr, MAC_SIZE) != 0) {
        return;
      }
      handle_game_update(msg->data);
      break;
    default:
      break;
  }
}
void IA() {
  while (1) {
    increment_strenght();
    if (client_mode)
      update_player_data();
    else
      update_game_data();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void rope_game_init() {
  lobby_manager_set_display_status_cb(NULL);
  lobby_manager_register_custom_cmd_recv_cb(on_receive_data_cb);
  clear_game_data();
  player_init();
  games_screens_module_show_rope_game_event(UPDATE_GAME_EVENT);
  const esp_timer_create_args_t timer_args = {.callback = &decrement_strenght,
                                              .name = "example_timer"};

  esp_err_t ret = esp_timer_create(&timer_args, &timer_handle);
  if (ret != ESP_OK) {
    ESP_LOGE("Timer", "Failed to create timer: %s", esp_err_to_name(ret));
    return;
  }
  menu_screens_set_app_state(true, rope_game_input);
  xTaskCreate(IA, "IA", 2048, NULL, 10, NULL);
}

void rope_game_input(button_event_t button_pressed) {
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
  const char* button_name_str = button_names[button_name];
  const char* button_event_str = button_events_name[button_event];
  switch (button_name) {
    case BUTTON_LEFT:
      switch (button_event) {
        case BUTTON_PRESS_DOWN:
          menu_screens_set_app_state(false, NULL);
          menu_screens_exit_submenu();
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
