#include "ble_module.h"
#include "ajo_module.h"
#include "bt_spam.h"
#include "ctf_ble.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "menu_screens_modules.h"
#include "modules/ble/ble_screens_module.h"
#include "neopixels_events.h"
#include "neopixels_module.h"
#include "oled_screen.h"
#include "trackers_scanner.h"

static app_screen_state_information_t app_screen_state_information = {
    .in_app = false,
    .app_selected = 0,
};
static int trackers_count = 0;
static int device_selection = 0;
static int ctf_ble_score = 0;
static int ctf_ble_page = 0;
static char ctf_ble_flags[20];
static bool is_displaying = false;
static bool is_modal_displaying = false;
static bool ctf_ble_flag_received = false;
static tracker_profile_t* scanned_airtags = NULL;
static TaskHandle_t ble_task_display_records = NULL;

static void ble_module_app_selector();
static void ble_module_state_machine(button_event_t button_pressed);
static void ble_module_display_trackers_cb(tracker_profile_t record);
static void ble_module_task_start_trackers_display_devices();
static void ble_module_task_stop_trackers_display_devices();
static void ble_module_display_ctf_score();
static void ble_module_display_ctf_cb(int score, char* flags);

static void set_ble_color() {
  neopixels_set_pixels(MAX_LED_NUMBER, 0, 0, 50);  // BLUE
  neopixels_refresh();
}

static void set_ble_color_off() {
  neopixels_set_pixels(MAX_LED_NUMBER, 0, 0, 0);  // OFF
  neopixels_refresh();
}

static void set_ble_color_red() {
  neopixels_set_pixels(MAX_LED_NUMBER, 50, 0, 0);  // RED
  neopixels_refresh();
}

static void set_ble_flag_color() {
  neopixels_set_pixels(MAX_LED_NUMBER, 0, 50, 0);  // GREEN
  neopixels_refresh();
}

static void set_waiting_color() {
  while (true) {
    if (ctf_ble_flag_received) {
      continue;
    }
    neopixels_set_pixels(0, 0, 0, 50);                                 // BLUE
    neopixels_set_pixels(1, 0, 0, (ctf_ble_score % 2 == 0) ? 50 : 0);  // BLUE
    neopixels_set_pixels(2, 0, 0, 50);                                 // BLUE
    neopixels_set_pixels(3, 0, 0, (ctf_ble_score % 2 == 0) ? 50 : 0);  // BLUE
    neopixels_refresh();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void ble_module_begin(int app_selected) {
  ESP_LOGI(TAG_BLE_MODULE, "Initializing ble module screen state machine");
  app_screen_state_information.app_selected = app_selected;

  menu_screens_set_app_state(true, ble_module_state_machine);
  oled_screen_clear();
  ble_module_app_selector();
};

static void ble_module_app_selector() {
  neopixel_events_run_event(neopixel_scanning_event);
  switch (app_screen_state_information.app_selected) {
    case MENU_BLUETOOTH_TRAKERS_SCAN:
      trackers_scanner_register_cb(ble_module_display_trackers_cb);
      ble_module_task_start_trackers_display_devices();
      trackers_scanner_start();
      break;
    case MENU_BLUETOOTH_SPAM:
      bool is_ajo = ajo_module_display_animation();
      if (is_ajo) {
        bt_spam_register_cb(ble_screens_display_scanning_text_ajo);
      } else {
        ble_screens_display_scanning_animation();
        bt_spam_register_cb(ble_screens_display_scanning_text);
      }
      bt_spam_app_main();
      break;
    case MENU_BLUETOOTH_CTF:
      xTaskCreate(set_waiting_color, "waiting_color", 2048, NULL, 10, NULL);
      ctf_ble_flag_register_cb(ble_module_display_ctf_cb);
      set_ble_color();
      ctf_ble_module_begin();
      ctf_ble_show_intro();
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      ble_module_display_ctf_score();
      break;
    default:
      break;
  }
}

static void ble_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;
  if (button_event != BUTTON_SINGLE_CLICK &&
      button_event != BUTTON_LONG_PRESS_HOLD) {
    return;
  }

  ESP_LOGI(TAG_BLE_MODULE, "BLE engine state machine from team: %d %d",
           button_name, button_event);
  switch (app_screen_state_information.app_selected) {
    case MENU_BLUETOOTH_TRAKERS_SCAN:
      ESP_LOGI(TAG_BLE_MODULE, "Bluetooth scanner entered");
      switch (button_name) {
        case BUTTON_LEFT:
          ESP_LOGI(TAG_BLE_MODULE, "Button left pressed");
          screen_module_set_screen(MENU_BLUETOOTH_TRAKERS_SCAN);
          set_ble_color_off();
          esp_restart();
          break;
        case BUTTON_RIGHT:
          ESP_LOGI(TAG_BLE_MODULE, "Button right pressed - Option selected: %d",
                   device_selection);
          if (is_modal_displaying) {
            break;
          }
          is_modal_displaying = true;
          ble_screens_display_modal_trackers_profile(
              scanned_airtags[device_selection]);
          break;
        case BUTTON_UP:
          ESP_LOGI(TAG_BLE_MODULE, "Button up pressed");
          device_selection = (device_selection == 0) ? 0 : device_selection - 1;
          break;
        case BUTTON_DOWN:
          ESP_LOGI(TAG_BLE_MODULE, "Button down pressed");
          device_selection = (device_selection == (trackers_count - 1))
                                 ? device_selection
                                 : device_selection + 1;
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    case MENU_BLUETOOTH_SPAM:
      switch (button_name) {
        case BUTTON_LEFT:
          screen_module_set_screen(MENU_BLUETOOTH_SPAM);
          set_ble_color_off();
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
    case MENU_BLUETOOTH_CTF:
      switch (button_name) {
        case BUTTON_LEFT:
          screen_module_set_screen(MENU_BLUETOOTH_CTF);
          set_ble_color_off();
          esp_restart();
          break;
        case BUTTON_RIGHT:
        case BUTTON_UP:
          ctf_ble_page = (ctf_ble_page == 0) ? 2 : ctf_ble_page - 1;
          ble_module_display_ctf_score();
          break;
        case BUTTON_DOWN:
          ctf_ble_page = (ctf_ble_page == 2) ? 0 : ctf_ble_page + 1;
          ble_module_display_ctf_score();
          break;
        case BUTTON_BOOT:
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void ble_module_display_ctf_score() {
  oled_screen_clear();
  uint8_t device_eui64[8] = {0};
  esp_read_mac(device_eui64, ESP_MAC_BT);
  char score_text[20];
  snprintf(score_text, 20, "BLECTF SCORE: %d", ctf_ble_score);
  oled_screen_display_text_center(score_text, 0, OLED_DISPLAY_INVERT);

  if (ctf_ble_page == 0) {
    oled_screen_display_text_center(" MAC: ", 1, OLED_DISPLAY_NORMAL);
    char mac_address[30];
    sprintf(mac_address, "%02X:%02X:%02X", device_eui64[0], device_eui64[1],
            device_eui64[2]);
    oled_screen_display_text_center(mac_address, 2, OLED_DISPLAY_NORMAL);
    sprintf(mac_address, "%02X:%02X:%02X", device_eui64[3], device_eui64[4],
            device_eui64[5]);
    oled_screen_display_text_center(mac_address, 3, OLED_DISPLAY_NORMAL);
  } else if (ctf_ble_page == 1) {
    oled_screen_display_text_center("Showing: 0-10", 1, OLED_DISPLAY_NORMAL);
    char first_flags[20];
    char second_flags[20];
    sprintf(first_flags, "%c|%c|%c|%c|%c|", ctf_ble_flags[0], ctf_ble_flags[1],
            ctf_ble_flags[2], ctf_ble_flags[3], ctf_ble_flags[4]);
    sprintf(second_flags, "%c|%c|%c|%c|%c|", ctf_ble_flags[5], ctf_ble_flags[6],
            ctf_ble_flags[7], ctf_ble_flags[8], ctf_ble_flags[9]);
    oled_screen_display_text_center(first_flags, 2, OLED_DISPLAY_NORMAL);
    oled_screen_display_text_center(second_flags, 3, OLED_DISPLAY_NORMAL);
  } else {
    oled_screen_display_text_center("Showing: 10-20", 1, OLED_DISPLAY_NORMAL);
    char third_flags[20];
    char fourth_flags[20];
    sprintf(third_flags, "%c|%c|%c|%c|%c|", ctf_ble_flags[10],
            ctf_ble_flags[11], ctf_ble_flags[12], ctf_ble_flags[13],
            ctf_ble_flags[14]);
    sprintf(fourth_flags, "%c|%c|%c|%c|%c|", ctf_ble_flags[15],
            ctf_ble_flags[16], ctf_ble_flags[17], ctf_ble_flags[18],
            ctf_ble_flags[19]);
    oled_screen_display_text_center(third_flags, 2, OLED_DISPLAY_NORMAL);
    oled_screen_display_text_center(fourth_flags, 3, OLED_DISPLAY_NORMAL);
  }
}

static void ble_module_display_ctf_cb(int score, char* flags) {
  oled_screen_clear();
  oled_screen_display_text_center("Data Recived", 0, OLED_DISPLAY_INVERT);
  ctf_ble_flag_received = true;

  if (score == ctf_ble_score) {
    set_ble_color_red();
    oled_screen_display_text_center("Wrong Flag", 2, OLED_DISPLAY_INVERT);
    oled_screen_display_text_center("Try Again", 3, OLED_DISPLAY_NORMAL);
  } else {
    set_ble_flag_color();
    oled_screen_display_text_center("Correct Flag", 2, OLED_DISPLAY_INVERT);
    oled_screen_display_text_center("Good Job", 3, OLED_DISPLAY_NORMAL);
  }
  ctf_ble_score = score;
  for (int i = 0; i < 20; i++) {
    ctf_ble_flags[i] = flags[i];
  }
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  oled_screen_clear();
  ctf_ble_flag_received = false;

  ble_module_display_ctf_score();
}

static void ble_module_display_trackers_cb(tracker_profile_t record) {
  int has_device = trackers_scanner_find_profile_by_mac(
      scanned_airtags, trackers_count, record.mac_address);
  if (has_device == -1) {
    trackers_scanner_add_tracker_profile(&scanned_airtags, &trackers_count,
                                         record.mac_address, record.rssi,
                                         record.name);
  } else {
    scanned_airtags[has_device].rssi = record.rssi;
    if (is_modal_displaying) {
      ble_screens_display_modal_trackers_profile(
          scanned_airtags[device_selection]);
    }
  }
}

static void ble_module_create_task_trackers_display_devices() {
  while (is_displaying) {
    if (!is_modal_displaying) {
      ble_screens_display_trackers_profiles(scanned_airtags, trackers_count,
                                            device_selection);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void ble_module_task_start_trackers_display_devices() {
  is_displaying = true;
  xTaskCreate(ble_module_create_task_trackers_display_devices,
              "display_records", 2048, NULL, 10, &ble_task_display_records);
}

static void ble_module_task_stop_trackers_display_devices() {
  is_displaying = false;
  vTaskSuspend(ble_task_display_records);
}
