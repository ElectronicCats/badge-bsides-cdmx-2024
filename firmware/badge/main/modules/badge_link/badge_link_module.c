#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "badge_connect.h"
#include "badge_link_module.h"
#include "badge_link_screens_module.h"
#include "menu_screens_modules.h"

static const char* TAG = "badge_link_module";
TaskHandle_t badge_link_state_machine_task_handle;
TaskHandle_t badge_link_module_send_data_task_handle;

badge_link_screens_status_t badge_link_status = BADGE_LINK_SCANNING;
badge_link_screens_status_t badge_link_status_previous =
    BADGE_LINK_UNLOCK_FEATURE;

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

// Check badge_connect_recv_msg_t struct in badge_connect.h to see what you can
// get from the received message
void badge_link_receive_data_cb(badge_connect_recv_msg_t* msg) {
  char* data = (char*) msg->data;
  data[msg->data_size] = '\0';
  ESP_LOGI(TAG, "Received data: %s", data);

  bool is_hello_world = strcmp(data, "Hello world") == 0;

  if (is_hello_world && !msg->badge_type.is_bsides) {
    badge_link_status = BADGE_LINK_FOUND;
    badge_connect_deinit();
  }

  printf("RSSI: %d\n", msg->rx_ctrl->rssi);
  printf("Badge BSides: %s\n", msg->badge_type.is_bsides ? "true" : "false");
  printf("Badge DragonJAR: %s\n",
         msg->badge_type.is_dragonjar ? "true" : "false");
  printf("Badge Ekoparty: %s\n",
         msg->badge_type.is_ekoparty ? "true" : "false");
  printf("Badge BugCON: %s\n", msg->badge_type.is_bugcon ? "true" : "false");
}

void badge_link_module_send_data() {
  static uint8_t timeout = 0;
  timeout++;  // Increased every 100ms

  if (timeout / 10 > 15) {
    badge_link_status = BADGE_LINK_NOT_FOUND;
    return;
  }

  if (badge_link_status == BADGE_LINK_SCANNING) {
    char* data = "Hello world";
    uint8_t* addr = ESPNOW_ADDR_BROADCAST;  // Send to all badges
    badge_connect_send(addr, data, strlen(data));
  }
}

void badge_link_keyboard_cb(button_event_t button_pressed) {
  // >> 4 to get the button number
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  // & 0x0F to get the event number without the mask
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;

  switch (button_name) {
    case BUTTON_LEFT:
      if (button_event == BUTTON_PRESS_DOWN) {
        badge_link_module_exit();
      }
      break;
    case BUTTON_RIGHT:
      if (button_event == BUTTON_PRESS_DOWN) {
        // badge_link_status = BADGE_LINK_EXIT;
      }
      break;
    default:
      break;
  }
}

void badge_link_reset_status() {
  badge_link_status = BADGE_LINK_SCANNING;
  badge_link_status_previous = BADGE_LINK_EXIT;
}

void badge_link_state_machine_task(void* pvParameters) {
  badge_link_reset_status();

  while (true) {
    badge_link_module_send_data();

    if (badge_link_status != badge_link_status_previous) {
      ESP_LOGI(TAG, "Badge link status: %s",
               badge_link_status_strings[badge_link_status]);
      badge_link_screens_module_display_status(badge_link_status);
      badge_link_status_previous = badge_link_status;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void badge_link_module_begin() {
  ESP_LOGI(TAG, "Badge link module begin");
  menu_screens_set_app_state(true, badge_link_keyboard_cb);
  wifi_init();  // Needed to work with espnow
  badge_connect_init();
  badge_connect_register_recv_cb(badge_link_receive_data_cb);
  // Set the badge type to BSides, DragonJAR, Ekoparty, or BugCon
  // See README.md or badge_connect.h for more information
  badge_connect_set_bsides_badge();
  // badge_connect_set_bugcon_badge();
  xTaskCreate(badge_link_state_machine_task, "badge_link_state_machine_task",
              4096, NULL, 4, &badge_link_state_machine_task_handle);
}

void badge_link_module_exit() {
  vTaskDelete(badge_link_state_machine_task_handle);
  vTaskDelete(badge_link_module_send_data_task_handle);
  badge_connect_deinit();
  menu_screens_set_app_state(false, NULL);
  menu_screens_exit_submenu();
}
