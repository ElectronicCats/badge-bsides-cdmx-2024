#include "badge_link_screens_module.h"
#include "oled_screen.h"

void badge_link_screens_module_scan_task(void* pvParameters) {
  oled_screen_clear();
  oled_screen_display_text_center("Looking for", 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("other badges", 1, OLED_DISPLAY_NORMAL);

  while (true) {
    // Loading bar using spaces
    for (int i = 0; i < 128; i++) {
      oled_screen_display_text(" ", i, 3, OLED_DISPLAY_INVERT);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void badge_link_screens_module_display_status(
    badge_link_screens_status_t status) {
  switch (status) {
    case BADGE_LINK_SCANNING:
      xTaskCreate(badge_link_screens_module_scan_task,
                  "badge_link_screens_module_scan_task", 2048, NULL, 5,
                  &badge_link_screens_module_scan_task_handle);
      break;
    case BADGE_LINK_FOUND:
      oled_screen_clear();
      oled_screen_display_text_center("Badge found", 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    case BADGE_LINK_NOT_FOUND:
      oled_screen_clear();
      oled_screen_display_text("Badge not found", 0, 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    case BADGE_LINK_UNLOCK_FEATURE:
      oled_screen_clear();
      oled_screen_display_text_center("WiFi feature", 0, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("unlocked!", 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    default:
      break;
  }
}
