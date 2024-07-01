#include <stdio.h>
#include "ajo_module.h"
#include "cat_console.h"
#include "catdos_module.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "keyboard_module.h"
#include "leds.h"
#include "menu_screens_modules.h"
#include "open_thread.h"
#include "preferences.h"

static const char* TAG = "main";

void reboot_counter() {
  int32_t counter = preferences_get_int("reboot_counter", 0);
  ESP_LOGI(TAG, "Reboot counter: %ld", counter);
  counter++;
  preferences_put_int("reboot_counter", counter);
}

void app_main(void) {
  // ESP_ERROR_CHECK(esp_event_loop_create_default());

  leds_init();
  preferences_begin();
  menu_screens_begin();
  keyboard_module_begin();
  // menu_screens_display_menu();
  reboot_counter();
  leds_off();

  ajo_module_init();
  // cat_console_begin();

  bool is_ajo = ajo_module_get_state();
  ESP_LOGI(TAG, "AJO Module State: %d", is_ajo);
  int last_layer = preferences_get_int("MENUNUMBER", 99);
  if (last_layer == 99) {
    menu_screens_display_menu();
    show_logo();
  } else {
    screen_module_set_screen(last_layer);
    menu_screens_display_menu();
    preferences_put_int("MENUNUMBER", 99);
  }
}
