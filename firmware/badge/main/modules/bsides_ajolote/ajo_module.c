#include "ajo_module.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oled_screen.h"
#include "preferences.h"

static int ajolote_state_count = 0;
static void ajo_module_display_animation_unlocked();
static void ajo_module_gpio_init(uint32_t gpio_num, uint8_t mask);
static void ajo_module_gpio_event_cb(void* arg, void* data);
static TaskHandle_t ajo_task = NULL;
static bool running = false;

static void ajo_module_gpio_init(uint32_t gpio_num, uint8_t mask) {
  button_config_t btn_cfg = {
      .type = BUTTON_TYPE_GPIO,
      .gpio_button_config =
          {
              .gpio_num = gpio_num,
              .active_level = GPIO_SHITTY_ACTIVE_LEVEL,
          },
  };
  button_handle_t btn = iot_button_create(&btn_cfg);
  assert(btn);
  esp_err_t err =
      iot_button_register_cb(btn, BUTTON_PRESS_DOWN, ajo_module_gpio_event_cb,
                             (void*) (BUTTON_PRESS_DOWN | mask));
  err |= iot_button_register_cb(btn, BUTTON_PRESS_UP, ajo_module_gpio_event_cb,
                                (void*) (BUTTON_PRESS_UP | mask));
  err |=
      iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, ajo_module_gpio_event_cb,
                             (void*) (BUTTON_PRESS_REPEAT | mask));
  err |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT_DONE,
                                ajo_module_gpio_event_cb,
                                (void*) (BUTTON_PRESS_REPEAT_DONE | mask));
  err |=
      iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, ajo_module_gpio_event_cb,
                             (void*) (BUTTON_SINGLE_CLICK | mask));
  err |=
      iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, ajo_module_gpio_event_cb,
                             (void*) (BUTTON_DOUBLE_CLICK | mask));
  err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START,
                                ajo_module_gpio_event_cb,
                                (void*) (BUTTON_LONG_PRESS_START | mask));
  err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_HOLD,
                                ajo_module_gpio_event_cb,
                                (void*) (BUTTON_LONG_PRESS_HOLD | mask));
  err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP,
                                ajo_module_gpio_event_cb,
                                (void*) (BUTTON_LONG_PRESS_UP | mask));
  ESP_ERROR_CHECK(err);
}

/**
 * @brief GPIO event callback
 *
 * @param void* arg
 * @param void* data
 *
 * @return void
 */
static void ajo_module_gpio_event_cb(void* arg, void* data) {
  uint8_t button_event =
      ((button_event_t) data) &
      0x0F;  // & 0x0F to get the event number without the mask

  switch (button_event) {
    case BUTTON_PRESS_DOWN:
      printf("[AJO] Pulse press down\n");
      if (!preferences_get_bool("ajounlock", false)) {
        ajolote_state_count++;
        if (ajolote_state_count == 10) {
          preferences_put_bool("ajounlock", true);
          ajo_module_display_animation_unlocked();
        }
      }
      break;
    case BUTTON_PRESS_UP:
      printf("[AJO] Pulse press up\n");
      break;
    default:
      printf("[AJO_DEBU] Pulse %d", button_event);
      break;
  }
}

bool ajo_module_get_state() {
  return preferences_get_bool("ajounlock", false);
}

void ajo_module_init(void) {
  ajo_module_gpio_init(GPIO_IN_SHITTY, GPIO_IN_SHITTY_MASK);
}

static void ajo_module_display_animation_unlocked() {
  oled_screen_clear(OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("Ajolote", 1, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("UNLOCKED!", 2, OLED_DISPLAY_NORMAL);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  ajo_module_display_animation();
  vTaskDelay(6000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void ajo_module_animation_task() {
  running = true;
  while (running) {
    for (int i = 0; i < ajolote_allArray_LEN; i++) {
      oled_screen_display_bitmap(ajolote_allArray[i], 0, 8, 128, 32,
                                 OLED_DISPLAY_NORMAL);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

bool ajo_module_display_animation() {
  bool ajolote_unlocked = preferences_get_bool("ajounlock", false);
  if (!ajolote_unlocked) {
    return false;
  }
  if (running) {
    return true;
  }
  xTaskCreate(ajo_module_animation_task, "ajo_task", 4096, NULL, 5, &ajo_task);
  return true;
}

void ajo_module_delete_task() {
  running = false;
  vTaskDelete(ajo_task);
}
