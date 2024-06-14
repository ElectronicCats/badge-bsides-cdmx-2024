/**
 * Badge Connect library for BSides, DragonJAR, Ekoparty, and BugCon badges
 * by Francisco Torres, Electronic Cats (https://electroniccats.com/)
 * Date: June 2024
 *
 * This library provides a way to communicate between badges using the ESPNOW
 * protocol.
 *
 * Development environment specifics:
 * IDE: Visual Studio Code + ESP-IDF v5.4-dev-78-gd4cd437ede
 * Hardware Platform:
 * Tested on ESP32-C6, but should work on any ESP32
 *
 * Electronic Cats invests time and resources providing this open source code,
 * please support Electronic Cats and open-source hardware by purchasing
 * products from Electronic Cats!
 *
 * This code is beerware; if you see me (or any other Electronic Cats
 * member) at the local, and you've found our code helpful,
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_wifi.h"

typedef struct {
  bool is_bsides;
  bool is_dragonjar;
  bool is_ekoparty;
  bool is_bugcon;
} badge_type_t;

typedef struct {
  uint8_t* src_addr;
  void* data;
  size_t data_size;
  wifi_pkt_rx_ctrl_t* rx_ctrl;
  badge_type_t badge_type;
} badge_connect_recv_msg_t;

typedef void (*badge_connect_recv_cb_t)(badge_connect_recv_msg_t* msg);

/**
 * @brief Initialize the badge connect module
 *
 * @return void
 */
void badge_connect_init();

void badge_connect_deinit();

/**
 * @brief Set the badge type to BSides
 *
 * @return void
 */
void badge_connect_set_bsides_badge();

/**
 * @brief Set the badge type to DragonJAR
 *
 * @return void
 */
void badge_connect_set_dragonjar_badge();

/**
 * @brief Set the badge type to Ekoparty
 *
 * @return void
 */
void badge_connect_set_ekoparty_badge();

/**
 * @brief Set the badge type to BugCon
 *
 * @return void
 */
void badge_connect_set_bugcon_badge();

/**
 * @brief Send data to the badge connect module
 *
 * @param data The data to send
 * @param data_size The size of the data
 *
 * @return void
 */
void badge_connect_send(uint8_t* addr, void* data, size_t data_size);

/**
 * @brief Register a callback function to receive data
 *
 * @param cb The callback function
 *
 * @return void
 */
void badge_connect_register_recv_cb(badge_connect_recv_cb_t cb);
