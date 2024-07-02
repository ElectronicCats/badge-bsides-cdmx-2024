#include "badge_link_screens_module.h"
#include "oled_screen.h"

void badge_link_screens_module_start() {
  oled_screen_clear();
  oled_screen_display_text_center("Looking for", 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("other badges", 1, OLED_DISPLAY_NORMAL);
}
