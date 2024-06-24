#include "games_module.h"
#include "esp_log.h"
#include "games_screens_module.h"
#include "led_events.h"
#include "lobby_manager.h"
#include "menu_screens_modules.h"
#include "oled_screen.h"
#include "rope_game.h"

app_screen_state_information_t app_screen_state_information = {
    .in_app = false,
    .app_selected = 0,
};

void games_module_app_selector();
void games_module_state_machine(button_event_t button_pressed);

void games_module_begin(int app_selected) {
  app_screen_state_information.app_selected = app_selected;
  menu_screens_set_app_state(true, games_module_state_machine);
  oled_screen_clear(OLED_DISPLAY_NORMAL);
  lobby_manager_set_display_status_cb(games_screens_module_show_lobby_state);
  lobby_manager_init();
}
void games_module_state_machine(button_event_t button_pressed) {
  uint8_t button_name = button_pressed >> 4;
  uint8_t button_event = button_pressed & 0x0F;

  ESP_LOGI(TAG_GAMES_MODULE, "Games engine state machine from team: %d %d",
           button_name, button_event);
  switch (app_screen_state_information.app_selected) {
    case MENU_GAMES:
      switch (button_name) {
        case BUTTON_RIGHT:
          switch (button_event) {
            case BUTTON_PRESS_DOWN:
              rope_game_init();
              break;
            case BUTTON_PRESS_UP:
              break;
          }
          break;
        case BUTTON_LEFT:
          switch (button_event) {
            case BUTTON_PRESS_DOWN:
              lobby_manager_deinit();
              menu_screens_set_app_state(false, NULL);
              menu_screens_exit_submenu();
              break;
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
