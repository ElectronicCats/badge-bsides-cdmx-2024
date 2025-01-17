#include "badge_link_screens_module.h"
#include "oled_screen.h"

epd_bitmap_logo_index_t found_badge_logo_index = LOGO_BSIDES;

void badge_link_screens_module_scan_task(void* pvParameters) {
  oled_screen_clear();
  oled_screen_display_text_center("Buscando", 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("badges", 1, OLED_DISPLAY_NORMAL);

  while (true) {
    // Loading bar using spaces
    for (int i = 0; i < 128; i++) {
      oled_screen_display_text(" ", i, 3, OLED_DISPLAY_INVERT);
      vTaskDelay(117 / portTICK_PERIOD_MS);
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
    case BADGE_LINK_BRING_IT_CLOSER:
      oled_screen_clear_line(0, 0, OLED_DISPLAY_NORMAL);
      oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text("Acerca el badge", 1, 0, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_FOUND_TEXT:
      oled_screen_clear();
      oled_screen_display_text_center("Badge", 0, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("encontrado!", 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    case BADGE_LINK_FOUND_LOGO:
      oled_screen_clear();
      oled_screen_display_bitmap(epd_logo_bitmaps[found_badge_logo_index], 0, 0,
                                 128, 32, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_NOT_FOUND:
      oled_screen_clear();
      oled_screen_display_text_center("Badge no", 0, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("encontrado!", 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    case BADGE_LINK_UNLOCK_FEATURE:
      oled_screen_clear();
      oled_screen_display_text("Funcion de WiFi", 0, 0, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("desbloqueada!", 1, OLED_DISPLAY_NORMAL);
      oled_screen_display_text_center("Ok", 3, OLED_DISPLAY_INVERT);
      break;
    default:
      break;
  }
}

void badge_link_screens_module_set_found_badge_logo(
    epd_bitmap_logo_index_t new_found_badge_logo_index) {
  found_badge_logo_index = new_found_badge_logo_index;
}
