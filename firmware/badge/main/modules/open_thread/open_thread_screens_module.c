#include <string.h>
#include "oled_screen.h"
void open_thread_screens_display_broadcast_mode(uint8_t ch) {
  oled_screen_clear_line(0, 0, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("BroadCast Mode", 0, OLED_DISPLAY_NORMAL);
  char* str = (char*) malloc(13);
  sprintf(str, "Channel %d", ch);
  oled_screen_clear_line(0, 3, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center(str, 3, OLED_DISPLAY_NORMAL);
  free(str);
}

void open_thread_screens_show_new_message(uint8_t* msg) {
  oled_screen_clear_line(0, 1, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center("New Message", 1, OLED_DISPLAY_INVERT);
  char* str = (char*) malloc(15);
  sprintf(str, "Counter: %d", *msg);
  oled_screen_clear_line(0, 2, OLED_DISPLAY_NORMAL);
  oled_screen_display_text_center(str, 2, OLED_DISPLAY_NORMAL);
  free(str);
}
