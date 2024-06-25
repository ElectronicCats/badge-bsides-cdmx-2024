#include <stdio.h>
#include <string.h>
#include "badge_connect.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "espnow.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAC_SIZE        6
#define MAX_PLAYERS_NUM 4
#define PING_TIMEOUT_MS 4000
#define RSSI_FILTER     (-100)

typedef enum {
  PING_CMD,
  PING_RESPONSE_CMD,
  ADVERTISE_CMD,
  JOIN_REQUEST_CMD,
  JOIN_RESPONSE_CMD
} commands_t;

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
} ping_response_message_t;

typedef struct {
  uint8_t mac[MAC_SIZE];
  bool online;
  uint8_t player_id;
  // badge type?
  // others
} player_inf_t;

uint8_t my_mac[MAC_SIZE];
uint8_t host_mac[MAC_SIZE];

bool client_mode = 0;
int8_t my_client_id = 0;
uint8_t players_count;  // still unused
uint8_t my_host_level;
uint8_t host_level = 0;
player_inf_t players[MAX_PLAYERS_NUM];

esp_timer_handle_t ping_timer;
bool ping_timeout = false;

TaskHandle_t advertiser_task_handler;
TaskHandle_t games_unlocker_task_handler;

void send_join_request_response(uint8_t* mac, uint8_t idx);
void send_join_request();
void send_ping_response(uint8_t* mac);
bool is_client_my_client(uint8_t* mac);

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
  ping_timeout = true;
}
void send_ping_request(uint8_t* mac) {
  printf("send_ping_request\n");
  ping_message_t ping_msg = {.cmd = PING_CMD};
  badge_connect_send(mac, &ping_msg, sizeof(ping_msg));
  esp_timer_start_once(ping_timer, PING_TIMEOUT_MS * 1000);
}
void handle_ping_request(uint8_t* mac) {
  printf("handle_ping_request\n");
  send_ping_response(mac);
}
////////////////////////////////////////////////////////////////////////////////////////
void send_ping_response(uint8_t* mac) {
  printf("send_ping_response\n");
  ping_response_message_t ping_response_msg = {
      .cmd = PING_RESPONSE_CMD,
      .client_mode = client_mode,
      .my_child = is_client_my_client(mac)};

  badge_connect_send(mac, &ping_response_msg, sizeof(ping_response_msg));
  esp_timer_start_once(ping_timer, PING_TIMEOUT_MS * 1000);
}
void handle_ping_response(badge_connect_recv_msg_t* msg) {
  printf("handle_ping_response\n");
  ping_response_message_t* ping_response_msg =
      (ping_response_message_t*) msg->data;
  if (ping_response_msg->client_mode || !ping_response_msg->my_child) {
    printf("host isnt host or not my host, listening...\n");
    memset(host_mac, 0, MAC_SIZE);
    client_mode = false;
    host_level = 0;
  }
  esp_timer_stop(ping_timer);
}
////////////////////////////////////////////////////////////////////////////////////////
bool is_client_my_client(uint8_t* mac) {
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
    if (memcmp(players[i].mac, mac, MAC_SIZE) == 0) {
      return true;
    }
  }
  return false;
}
bool add_new_player(uint8_t* mac) {
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
    if (memcmp(players[i].mac, mac, MAC_SIZE) == 0) {
      printf("PLayer already in this lobby\n");
      send_join_request_response(mac, i);
      return true;
    }
  }
  for (uint8_t i = 0; i < MAX_PLAYERS_NUM; i++) {
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
uint8_t get_players_count() {
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
}

////////////////////////////////////////////////////////////////////////////////////////
void send_advertise() {
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
  if (memcmp(mac, host_mac, MAC_SIZE) == 0) {
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
  client_mode = true;
}
////////////////////////////////////////////////////////////////////////////////////////
void unlock_games(uint8_t players_cnt) {}

void games_unlocker_task() {
  while (1) {
    unlock_games(get_players_count());
    vTaskDelay(500);
  }
}
////////////////////////////////////////////////////////////////////////////////////////
void advertiser_task() {
  while (1) {
    if (!client_mode) {
      if (get_players_count() < MAX_PLAYERS_NUM) {
        send_advertise();
      } else {
        printf("Lobby is full");
      }
      vTaskDelay(pdMS_TO_TICKS(2000));
    } else  // Quiza se pueda cambiar por un task
    {
      printf("+ client_mode-> Player%d\t Host: ", my_client_id);
      print_mac_address(host_mac);
      printf("\n");
      if (ping_timeout)  // TODO: ping retries
      {
        printf("Host timeout, listening for new host\n");
        memset(host_mac, 0, MAC_SIZE);
        host_level = 0;
        ping_timeout = false;
        client_mode = false;
        continue;
      }
      send_ping_request(host_mac);
      vTaskDelay(pdMS_TO_TICKS(PING_TIMEOUT_MS + 1000));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
void receive_data_cb(badge_connect_recv_msg_t* msg) {
  uint8_t cmd = *((uint8_t*) msg->data);
  // printf("CMD: %d\n", cmd);
  // printf("RSSI: %d\n", msg->rx_ctrl->rssi);
  if (msg->rx_ctrl->rssi < RSSI_FILTER /* || !rx1 || !rx2 */) {
    printf("So Far or not connected to another badge\n");
    return;
  }
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
      break;
  }
}

void lobby_manager_init() {
  clear_players_table();
  badge_connect_init();
  badge_connect_register_recv_cb(receive_data_cb);
  badge_connect_set_bsides_badge();

  my_host_level = get_random_uint8();
  esp_wifi_get_mac(WIFI_IF_STA, my_mac);

  const esp_timer_create_args_t ping_timer_args = {
      .callback = &ping_timeout_handler, .name = "ping_timeout"};
  ESP_ERROR_CHECK(esp_timer_create(&ping_timer_args, &ping_timer));

  xTaskCreate(advertiser_task, "Advertiser Task", 2048, NULL, 10,
              &advertiser_task_handler);
  xTaskCreate(games_unlocker_task, "Games Unlocker Task", 2048, NULL, 10,
              &games_unlocker_task_handler);
}
