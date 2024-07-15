#include "lobby_manager.h"
#include <stdio.h>
#include "badge_connect.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "espnow.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#define BADGE_UNCONNECTED \
  (gpio_get_level(BADGE_IN_1) && gpio_get_level(BADGE_IN_2))

#define DESACTIVAR_PRINT

#ifdef DESACTIVAR_PRINT
  #define printf(fmt, ...) ((void) 0)
#endif

typedef enum {
  PING_CMD,
  PING_RESPONSE_CMD,
  ADVERTISE_CMD,
  JOIN_REQUEST_CMD,
  JOIN_RESPONSE_CMD
} commands_t;

typedef enum {
  PING_NONE,
  PING_WAITING,
  PING_SUCCESS,
  PING_TIMEOUT,
} ping_status_t;

typedef struct {
  uint8_t cmd;
  uint8_t host_level;
} advertise_message_t;

typedef struct {
  uint8_t cmd;
} join_message_t;

typedef struct {
  uint8_t cmd;
  uint8_t host_level;
  uint8_t idx;
} join_response_message_t;

typedef struct {
  uint8_t cmd;
} ping_message_t;

typedef struct {
  uint8_t cmd;
  bool client_mode;
  bool my_child;
  player_inf_t players[MAX_PLAYERS_NUM];
} ping_response_message_t;

uint8_t my_mac[MAC_SIZE];

uint8_t players_count;  // still unused
uint8_t my_host_level;
uint8_t host_level = 0;

esp_timer_handle_t ping_timer;
uint8_t ping_status = 0;
uint8_t ping_attempt = 0;
uint8_t ping_id = 1;

bool advertiser_state = false;

display_status_cb_t display_event_cb = NULL;
badge_connect_recv_cb_t custom_cmd_recv_cb = NULL;

void send_join_request_response(uint8_t* mac, uint8_t idx);
void send_join_request();
void send_ping_response(uint8_t* mac);
bool is_host_my_host(uint8_t* mac);
bool is_client_my_client(uint8_t* mac);
void client_mode_exit();
void on_ping_timeout();
int8_t get_client_id(mac);

void display_state(uint8_t event);

uint8_t get_random_uint8() {
  uint32_t entropy = esp_random();
  uint64_t time_since_boot = esp_timer_get_time();
  uint32_t seed = (entropy ^ time_since_boot) & 0xFFFFFFFF;
  srand(seed);

  return rand() % 256;
}

void print_mac_address(uint8_t mac[MAC_SIZE]) {
  for (int i = 0; i < MAC_SIZE; i++) {
    printf("%02X", mac[i]);
    if (i < MAC_SIZE - 1) {
      printf(":");
    }
  }
}
void print_players_table() {
  printf("+++++++++++++++++++++++++++++++++++++++++\n");
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
    printf(" Player IDX: %d\n", players[i].player_id);
    printf("  MAC: ");
    print_mac_address(players[i].mac);
    printf("\n");
    printf("  Is online: %s\n", players[i].online ? "yes" : "no");
  }
  printf("+++++++++++++++++++++++++++++++++++++++++\n");
}
////////////////////////////////////////////////////////////////////////////////////////
void ping_timeout_handler() {
  if (ping_attempt < PING_ATTEMPTS) {
    ping_status = PING_NONE;
    ping_attempt++;
  } else {
    ping_status = PING_TIMEOUT;
    on_ping_timeout();
    ping_attempt = 0;
  }
}
void send_ping_request(uint8_t* mac) {
  printf("send_ping_request\n");
  ping_message_t ping_msg = {.cmd = PING_CMD};
  badge_connect_send(mac, &ping_msg, sizeof(ping_msg));
}
void handle_ping_request(uint8_t* mac) {
  printf("handle_ping_request\n");
  if (client_mode) {
    if (is_host_my_host(mac)) {
      send_ping_response(mac);
    }
  } else {
    if (is_client_my_client(mac)) {
      send_ping_response(mac);
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////
void send_ping_response(uint8_t* mac) {
  printf("send_ping_response\n");
  ping_response_message_t ping_response_msg = {
      .cmd = PING_RESPONSE_CMD,
      .client_mode = client_mode,
      .my_child = is_client_my_client(mac)};
  memcpy(ping_response_msg.players, players, sizeof(players));
  badge_connect_send(mac, &ping_response_msg, sizeof(ping_response_msg));
}
void handle_ping_response(badge_connect_recv_msg_t* msg) {
  printf("handle_ping_response\n");
  ping_response_message_t* ping_response_msg =
      (ping_response_message_t*) msg->data;
  if (client_mode) {
    if (ping_response_msg->client_mode || !ping_response_msg->my_child) {
      printf("host isnt host or not my host, listening...\n");
      client_mode_exit();
      return;
    }
    memcpy(players, ping_response_msg->players, sizeof(players));
    esp_timer_stop(ping_timer);
    printf("PING SUCCESS\n");
    print_players_table();
    ping_attempt = 0;
    ping_status = PING_SUCCESS;
  } else {
    if (is_client_my_client(msg->src_addr)) {
      esp_timer_stop(ping_timer);
      printf("PING SUCCESS\n");
      ping_attempt = 0;
      ping_status = PING_SUCCESS;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////
bool is_host_my_host(uint8_t* mac) {
  return memcmp(host_mac, mac, MAC_SIZE) == 0 ? true : false;
}
bool is_client_my_client(uint8_t* mac) {
  for (uint8_t i = 1; i < MAX_PLAYERS_NUM; i++) {
    if (memcmp(players[i].mac, mac, MAC_SIZE) == 0) {
      return true;
    }
  }
  return false;
}

bool add_new_player(uint8_t* mac) {
  int8_t client_id = get_client_id(mac);
  if (client_id != -1) {
    printf("PLayer already in this lobby\n");
    send_join_request_response(mac, client_id);
    return true;
  }
  for (uint8_t i = 1; i < MAX_PLAYERS_NUM; i++) {
    if (!players[i].online)  // Ajustar en la marcha
    {
      memcpy(players[i].mac, mac, MAC_SIZE);
      players[i].online = true;
      players[i].player_id = i;
      printf("Accept player\n");
      send_join_request_response(mac, i);
      return true;
    }
  }
  printf("Lobby is full\n");
  return false;
}
int8_t get_client_id(uint8_t* mac) {
  for (uint8_t i = 1; i < MAX_PLAYERS_NUM; i++) {
    if (memcmp(players[i].mac, mac, MAC_SIZE) == 0) {
      return i;
    }
  }
  return -1;
}
uint8_t get_clients_count() {
  uint8_t cnt = 0;
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
    if (players[i].online)  // Ajustar en la marcha
    {
      cnt++;
    }
  }
  return cnt;
}

void clear_players_table() {
  memset(&players, 0, sizeof(player_inf_t) * MAX_PLAYERS_NUM);
  memcpy(players[0].mac, my_mac, MAC_SIZE);
  players[0].online = 1;
  players[0].player_id = 0;
}
void clear_client(uint8_t player_id) {
  memset(&players[player_id], 0, sizeof(player_inf_t));
}

////////////////////////////////////////////////////////////////////////////////////////
void send_advertise() {
  if (BADGE_UNCONNECTED) {
    clear_players_table();
    return;
  }
  printf("send_advertise\t MAC:");
  print_mac_address(my_mac);
  printf("\n");
  print_players_table();
  advertise_message_t adv_msg;
  adv_msg.cmd = ADVERTISE_CMD;
  adv_msg.host_level = my_host_level;
  badge_connect_send(ESPNOW_ADDR_BROADCAST, &adv_msg, sizeof(adv_msg));
}

void handle_advertise(badge_connect_recv_msg_t* msg) {
  printf("handle_advertise\n");
  advertise_message_t* adv_msg = (advertise_message_t*) msg->data;
  if ((host_level == 0 ? my_host_level : host_level) < adv_msg->host_level) {
    send_join_request(msg->src_addr);
  } else if (my_host_level == adv_msg->host_level) {
    printf("Same host level, get new host level\n");
    my_host_level = get_random_uint8();
  }
}
////////////////////////////////////////////////////////////////////////////////////////
void send_join_request(uint8_t* mac) {
  if (is_host_my_host(mac)) {
    printf("already joined to this lobby\n");
    client_mode = true;
  } else {
    clear_players_table();
    printf("send_join_request\n");
    join_message_t join_msg = {.cmd = JOIN_REQUEST_CMD};
    badge_connect_send(mac, &join_msg, sizeof(join_msg));
  }
}

void handle_join_request(uint8_t* mac) {
  // printf("handle_join_request\n");
  add_new_player(mac);
}
////////////////////////////////////////////////////////////////////////////////////////
void send_join_request_response(uint8_t* mac, uint8_t idx) {
  // printf("send_join_request_response\n");
  join_response_message_t join_resp = {
      .cmd = JOIN_RESPONSE_CMD, .host_level = my_host_level, .idx = idx};
  badge_connect_send(mac, &join_resp, sizeof(join_resp));
}

void handle_join_request_response(badge_connect_recv_msg_t* msg) {
  // printf("handle_join_response\n");
  join_response_message_t* join_response_msg =
      (join_response_message_t*) msg->data;
  if ((host_level == 0 ? my_host_level : host_level) >
      join_response_msg->host_level) {
    printf("Avoid re-joining to weaker host lobby on multiple acceptances");
    return;
  }
  memcpy(host_mac, msg->src_addr, MAC_SIZE);
  host_level = join_response_msg->host_level;
  my_client_id = join_response_msg->idx;
  printf("Joined to lobby-> Player%d\n", my_client_id);
  ESP_LOGE("JOIN", "Joined to lobby-> Player%d\n", my_client_id);
  client_mode = true;
}

////////////////////////////////////////////////////////////////////////////////////////
void client_mode_exit() {
  printf("Client Mode Exit\n");
  memset(host_mac, 0, MAC_SIZE);
  clear_players_table();
  host_level = 0;
  client_mode = false;
  my_client_id = 0;
  ping_id = 1;
}
void on_ping_timeout() {
  if (client_mode) {
    printf("Host timeout, listening for new host\n");
    ESP_LOGE("PING", "TIMEOUT!!!");
    client_mode_exit();
  } else {
    ESP_LOGE("PING", "player%d is offline\n", ping_id);
    printf("player%d is offline\n", ping_id);
    clear_client(ping_id);
  }
}

void seek_next_online_client() {
  printf("seek_next_online_client %d\n", ping_id);
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
    ping_id = ++ping_id > MAX_PLAYERS_NUM ? 1 : ping_id;
    if (players[ping_id].online) {
      printf("Found at %d\n", ping_id);
      break;
    }
  }
}
void ping(uint8_t* mac) {
  if (ping_status == PING_NONE) {
    ping_status = PING_WAITING;
    send_ping_request(mac);
    esp_timer_start_once(ping_timer, PING_TIMEOUT_MS * 1000);
    // ESP_LOGI("LOBBY MANAGER", "ATTEMPT: %d\n", ping_attempt);
  }
  if (ping_status == PING_WAITING)
    return;
  if (ping_status == PING_TIMEOUT) {
  } else if (ping_status == PING_SUCCESS) {
  }
  ping_status = PING_NONE;
  if (!client_mode) {
    seek_next_online_client();
  } else {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void advertiser_task() {
  advertiser_state = true;
  while (advertiser_state) {
    if (!client_mode) {
      display_state(SHOW_CLIENTS);
      if (get_clients_count() < MAX_PLAYERS_NUM) {
        send_advertise();
      } else {
        printf("Lobby is full");
      }
      if (players[ping_id].online) {
        ping(players[ping_id].mac);
      } else {
        seek_next_online_client();
      }
    } else {
      display_state(CLIENT_STATE);
      printf("+ client_mode-> Player%d\t Host: ", my_client_id);
      print_mac_address(host_mac);
      printf("\n");
      ping(host_mac);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  ESP_LOGI("ADVERTISER TASK", "Task deleted");
  advertiser_task_handler = NULL;
  vTaskDelete(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
void configure_pins() {
  gpio_config_t io_conf;

  // Configure input pins
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1ULL << BADGE_IN_1) | (1ULL << BADGE_IN_2);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);

  // Configure output pins
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << BADGE_OUT_1) | (1ULL << BADGE_OUT_2);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  gpio_set_level(BADGE_OUT_1, 0);
  gpio_set_level(BADGE_OUT_2, 0);
}

void deconfigure_pins() {
  gpio_reset_pin(BADGE_IN_1);
  gpio_reset_pin(BADGE_IN_2);
  gpio_reset_pin(BADGE_OUT_1);
  gpio_reset_pin(BADGE_OUT_2);
}

void receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  // printf("CMD: %d\n", cmd);
  // printf("RSSI: %d\n", msg->rx_ctrl->rssi);
  if (cmd < 10) {
    if (msg->rx_ctrl->rssi < RSSI_FILTER || BADGE_UNCONNECTED) {
      // display_state(SHOW_UNCONNECTED);
      ESP_LOGE("UNCONNECTED", "CMD: %d SKIPPED\n", cmd);
      return;
    }
  }
  // ESP_LOGI("CONNECTED", "CMD: %d\n", cmd);
  switch (cmd) {
    case ADVERTISE_CMD:
      handle_advertise(msg);
      break;
    case JOIN_REQUEST_CMD:
      handle_join_request(msg->src_addr);
      break;
    case JOIN_RESPONSE_CMD:
      handle_join_request_response(msg);
      break;
    case PING_CMD:
      handle_ping_request(msg->src_addr);
      break;
    case PING_RESPONSE_CMD:
      handle_ping_response(msg);
      break;
    default:
      if (custom_cmd_recv_cb) {
        printf("UNRECOGNIZED CMD\n");
        custom_cmd_recv_cb(msg);
      }
      break;
  }
}

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

void lobby_manager_init() {
  configure_pins();
  wifi_init();
  badge_connect_init();
  badge_connect_register_recv_cb(receive_data_cb);
  badge_connect_set_bsides_badge();

  my_host_level = get_random_uint8();
  my_host_level = 255;
  esp_wifi_get_mac(WIFI_IF_STA, my_mac);
  clear_players_table();

  const esp_timer_create_args_t ping_timer_args = {
      .callback = &ping_timeout_handler, .name = "ping_timeout"};
  ESP_ERROR_CHECK(esp_timer_create(&ping_timer_args, &ping_timer));

  xTaskCreate(advertiser_task, "Advertiser Task", 4096, NULL, 10,
              &advertiser_task_handler);
}
void lobby_manager_deinit() {
  advertiser_state = false;
  vTaskDelay(pdMS_TO_TICKS(200));
  deconfigure_pins();

  if (ping_timer != NULL) {
    esp_timer_stop(ping_timer);
    esp_timer_delete(ping_timer);
  }
  clear_players_table();
  badge_connect_deinit();
}
void display_state(uint8_t event) {
  if (display_event_cb) {
    display_event_cb(event);
  }
}
void lobby_manager_set_display_status_cb(display_status_cb_t cb) {
  display_event_cb = cb;
}

void lobby_manager_register_custom_cmd_recv_cb(badge_connect_recv_cb_t cb) {
  custom_cmd_recv_cb = cb;
}

#undef DESACTIVAR_PRINT
