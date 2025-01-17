#include "zigbee_module.h"
#include "ajo_module.h"
#include "esp_log.h"
#include "ieee_sniffer.h"
#include "menu_screens_modules.h"
#include "neopixels_events.h"
#include "oled_screen.h"
#include "preferences.h"
#include "radio_selector.h"
#include "uart_sender.h"
#include "zigbee_screens_module.h"
#include "zigbee_switch.h"

app_screen_state_information_t app_screen_state_information = {
    .in_app = false,
    .app_selected = 0,
};
int current_channel = IEEE_SNIFFER_CHANNEL_DEFAULT;
int packet_count = 0;
TaskHandle_t zigbee_task_display_records = NULL;
TaskHandle_t zigbee_task_display_animation = NULL;
TaskHandle_t zigbee_task_sniffer = NULL;

static bool is_ajolote_unlocked = false;

void zigbee_module_app_selector();
void zigbee_module_state_machine(button_event_t button_pressed);

static void zigbee_module_display_records_cb(uint8_t* packet,
                                             uint8_t packet_length) {
  if (packet_count == 1000) {
    packet_count = 0;
  }
  packet_count++;
  zigbee_screens_display_scanning_text(packet_count);
  uart_sender_send_packet(packet, packet_length);
}
void zigbee_module_begin(int app_selected) {
  ESP_LOGI(TAG_ZIGBEE_MODULE, "Initializing ble module screen state machine");
  app_screen_state_information.app_selected = app_selected;

  menu_screens_set_app_state(true, zigbee_module_state_machine);
  oled_screen_clear(OLED_DISPLAY_NORMAL);
  zigbee_module_app_selector();
};

void zigbee_module_app_selector() {
  switch (app_screen_state_information.app_selected) {
    case MENU_ZIGBEE_SWITCH:
      radio_selector_set_zigbee_switch();
      zigbee_switch_set_display_status_cb(zigbee_screens_module_display_status);
      zigbee_switch_init();
      break;
    case MENU_ZIGBEE_SNIFFER:
      radio_selector_set_zigbee_sniffer();
      zigbee_screens_display_device_ad();
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      ieee_sniffer_register_cb(zigbee_module_display_records_cb);

      xTaskCreate(ieee_sniffer_begin, "ieee_sniffer_task", 4096, NULL, 5,
                  &zigbee_task_sniffer);

      bool ajo_unlock = ajo_module_display_animation();
      if (!ajo_unlock) {
        zigbee_screens_display_scanning_text(0);
      }
      neopixel_events_run_event(neopixel_scanning_event);
      break;
    default:
      break;
  }
}

void zigbee_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_ZIGBEE_MODULE, "Zigbee engine state machine from team: %d %d",
           button_name, button_event);
  switch (app_screen_state_information.app_selected) {
    case MENU_ZIGBEE_SWITCH:
      ESP_LOGI(TAG_ZIGBEE_MODULE, "Zigbee Switch Entered");
      switch (button_name) {
        case BUTTON_RIGHT:
          switch (button_event) {
            case BUTTON_PRESS_DOWN:
              if (zigbee_switch_is_light_connected()) {
                zigbee_screens_module_toogle_pressed();
              }
              break;
            case BUTTON_PRESS_UP:
              if (zigbee_switch_is_light_connected()) {
                zigbee_screens_module_toggle_released();
                zigbee_switch_toggle();
              }
              break;
          }
          break;
        case BUTTON_LEFT:
          switch (button_event) {
            case BUTTON_PRESS_DOWN:
              preferences_put_bool("zigbee_deinit", true);
              screen_module_set_screen(MENU_ZIGBEE_SWITCH);
              zigbee_switch_deinit();
              break;
          }
          break;
        default:
          break;
      }
      break;
    case MENU_ZIGBEE_SNIFFER:
      ESP_LOGI(TAG_ZIGBEE_MODULE, "Zigbee Sniffer Entered");
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_ZIGBEE_MODULE, "Button left pressed");
          ieee_sniffer_stop();
          vTaskDelete(zigbee_task_sniffer);
          menu_screens_set_app_state(false, NULL);
          screen_module_set_screen(MENU_ZIGBEE_SNIFFER);
          menu_screens_exit_submenu();
          esp_restart();
          break;
        case BUTTON_RIGHT:
        case BUTTON_UP:
        case BUTTON_DOWN:
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    default:
      break;
  }
}
