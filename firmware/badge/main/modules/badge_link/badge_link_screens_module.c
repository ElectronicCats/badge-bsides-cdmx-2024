#include "badge_link_screens_module.h"
#include "oled_screen.h"

void badge_link_screens_module_scan() {
  oled_screen_clear();
  oled_screen_display_text_center("Looking for", 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("other badges", 1, OLED_DISPLAY_NORMAL);
}

void badge_link_screens_module_display_status(
    badge_link_screens_status_t status) {
  switch (status) {
    case BADGE_LINK_SCANNING:
      badge_link_screens_module_scan();
      break;
    case BADGE_LINK_FOUND:
      oled_screen_clear();
      oled_screen_display_text_center("Badge found", 1, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_NOT_FOUND:
      oled_screen_clear();
      oled_screen_display_text_center("Badge not found", 1,
                                      OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_CONNECTING:
      oled_screen_clear();
      oled_screen_display_text_center("Connecting", 1, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_CONNECTED:
      oled_screen_clear();
      oled_screen_display_text_center("Connected", 1, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_DISCONNECTED:
      oled_screen_clear();
      oled_screen_display_text_center("Disconnected", 1, OLED_DISPLAY_NORMAL);
      break;
    case BADGE_LINK_UNLOCK_FEATURE:
      oled_screen_clear();
      oled_screen_display_text_center("Unlocking feature", 1,
                                      OLED_DISPLAY_NORMAL);
      break;
    default:
      break;
  }
}
