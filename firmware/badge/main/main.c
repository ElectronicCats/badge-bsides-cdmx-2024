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
#include "neopixels_module.h"
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
  neopixels_module_begin();

  neopixels_set_pixels(4, 30, 0, 30);
  neopixels_refresh();

  leds_init();
  preferences_begin();
  menu_screens_begin();
  keyboard_module_begin();
  reboot_counter();

  ajo_module_init();
  // cat_console_begin();

  bool is_ajo = ajo_module_get_state();
  ESP_LOGI(TAG, "AJO Module State: %d", is_ajo);

  neopixels_set_pixels(4, 0, 0, 0);
  neopixels_refresh();
}
