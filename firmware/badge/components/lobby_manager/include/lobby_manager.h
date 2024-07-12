#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "badge_connect.h"

#define MAC_SIZE        6
#define MAX_PLAYERS_NUM 5
#define PING_TIMEOUT_MS 90
#define PING_ATTEMPTS   5
#define RSSI_FILTER     (-100)

/**
 * @brief GPIOs for badge connection
 * Connect RX0 to TX1 and RX1 to TX0
 */
#define BADGE_IN_1  GPIO_NUM_4   // RX1
#define BADGE_IN_2  GPIO_NUM_17  // RX0
#define BADGE_OUT_1 GPIO_NUM_5   // TX1
#define BADGE_OUT_2 GPIO_NUM_16  // TX0

typedef struct {
  uint8_t mac[MAC_SIZE];
  bool online;
  uint8_t player_id;
  // badge type?
  // others
} player_inf_t;

player_inf_t players[MAX_PLAYERS_NUM];
int8_t my_client_id = 0;
uint8_t host_mac[MAC_SIZE];
bool client_mode = false;
TaskHandle_t advertiser_task_handler;

typedef enum {
  HOST_STATE = 0,
  CLIENT_STATE,
  SHOW_CLIENTS,
  SHOW_UNCONNECTED
} lobby_events_t;

typedef void (*display_status_cb_t)(uint8_t);
void lobby_manager_set_display_status_cb(display_status_cb_t cb);
void lobby_manager_register_custom_cmd_recv_cb(badge_connect_recv_cb_t cb);

void lobby_manager_init();
void lobby_manager_deinit();
uint8_t get_clients_count();

uint8_t get_random_uint8();
