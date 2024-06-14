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
#include <stdio.h>

#include "badge_connect.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "espnow.h"
#include "espnow_storage.h"
#include "espnow_utils.h"
#include "nvs_flash.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
  #include "esp_mac.h"
#endif

#define BSIDES_BADGE_MASK       0x01
#define DRAGONJARCON_BADGE_MASK 0x02
#define EKOPARTY_BADGE_MASK     0x04
#define BUGCON_BADGE_MASK       0x08

static const char* TAG = "badge_connect";

uint8_t badge_type = BSIDES_BADGE_MASK;

badge_connect_recv_cb_t badge_connect_recv_cb = NULL;

/**
 * @brief Receive data from the badge connect module
 *
 * @param src_addr The source address
 * @param payload The data payload
 * @param payload_size The size of the payload
 * @param rx_ctrl The receive control
 *
 * @return esp_err_t
 */
esp_err_t badge_connect_recv(uint8_t* src_addr,
                             void* payload,
                             size_t payload_size,
                             wifi_pkt_rx_ctrl_t* rx_ctrl);

void nvs_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void wifi_init() {
  esp_event_loop_create_default();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  nvs_init();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void badge_connect_init() {
  wifi_init();
  espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
  espnow_init(&espnow_config);
  espnow_set_config_for_data_type(ESPNOW_DATA_TYPE_DATA, true,
                                  badge_connect_recv);
}
void badge_connect_deinit() {
  espnow_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  ESP_LOGI(TAG, "Badge connect module deinitialized");
}

void badge_connect_set_bsides_badge() {
  badge_type = BSIDES_BADGE_MASK;
}

void badge_connect_set_dragonjar_badge() {
  badge_type = DRAGONJARCON_BADGE_MASK;
}

void badge_connect_set_ekoparty_badge() {
  badge_type = EKOPARTY_BADGE_MASK;
}

void badge_connect_set_bugcon_badge() {
  badge_type = BUGCON_BADGE_MASK;
}

void badge_connect_send(uint8_t* addr, void* data, size_t data_size) {
  uint8_t badge_type_size = 1;
  size_t payload_size = data_size + badge_type_size;
  uint8_t* payload = (uint8_t*) malloc(payload_size);
  if (!payload) {
    ESP_LOGE(TAG, "Failed to allocate memory for data");
    return;
  }

  // Payload pointer stores badge_type in the first byte and the actual data in
  // the rest of the bytes
  payload[0] = badge_type;
  memcpy(payload + badge_type_size, data, data_size);

  espnow_frame_head_t frame_head = {
      .retransmit_count = 5,
      .broadcast = true,
  };

  esp_err_t ret = espnow_send(ESPNOW_DATA_TYPE_DATA, addr, payload,
                              payload_size, &frame_head, portMAX_DELAY);
}

esp_err_t badge_connect_recv(uint8_t* src_addr,
                             void* payload,
                             size_t payload_size,
                             wifi_pkt_rx_ctrl_t* rx_ctrl) {
  ESP_PARAM_CHECK(src_addr);
  ESP_PARAM_CHECK(payload);
  ESP_PARAM_CHECK(payload_size);
  ESP_PARAM_CHECK(rx_ctrl);

  char* data = malloc(payload_size);
  if (!data) {
    ESP_LOGE(TAG, "Failed to allocate memory for received data");
    return ESP_FAIL;
  }

  uint8_t badge_type_size = 1;
  size_t data_size = payload_size - badge_type_size;
  uint8_t badge_type_value = ((uint8_t*) payload)[0];
  memcpy(data, payload + badge_type_size, data_size);

  badge_type_t badge_type = {
      .is_bsides = badge_type_value & BSIDES_BADGE_MASK,
      .is_dragonjar = badge_type_value & DRAGONJARCON_BADGE_MASK,
      .is_ekoparty = badge_type_value & EKOPARTY_BADGE_MASK,
      .is_bugcon = badge_type_value & BUGCON_BADGE_MASK,
  };

  badge_connect_recv_msg_t msg = {
      .src_addr = src_addr,
      .data = (void*) data,
      .data_size = data_size,
      .rx_ctrl = rx_ctrl,
      .badge_type = badge_type,
  };

  if (badge_connect_recv_cb) {
    badge_connect_recv_cb(&msg);
  }

  return ESP_OK;
}

void badge_connect_register_recv_cb(badge_connect_recv_cb_t cb) {
  badge_connect_recv_cb = cb;
}
