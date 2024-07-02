#include "badge_link_module.h"
#include "badge_link_screens_module.h"
#include "menu_screens_modules.h"

void badge_link_keyboard_cb(button_event_t button_pressed) {
  // >> 4 to get the button number
  uint8_t button_name = (((button_event_t) button_pressed) >> 4);
  // & 0x0F to get the event number without the mask
  uint8_t button_event = ((button_event_t) button_pressed) & 0x0F;
}

void badge_link_begin() {
  menu_screens_set_app_state(true, badge_link_keyboard_cb);
  badge_link_screens_module_start();
}
