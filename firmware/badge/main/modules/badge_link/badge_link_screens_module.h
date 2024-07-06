#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
  BADGE_LINK_SCANNING = 0,
  BADGE_LINK_FOUND,
  BADGE_LINK_NOT_FOUND,
  BADGE_LINK_UNLOCK_FEATURE,
  BADGE_LINK_EXIT,
} badge_link_screens_status_t;

const char* badge_link_status_strings[] = {"Scanning", "Found", "Not found",
                                           "Feature unlocked", "Exit"};

TaskHandle_t badge_link_screens_module_scan_task_handle = NULL;

/**
 * @brief Display the status screen
 *
 * @param status
 * @return void
 */
void badge_link_screens_module_display_status(
    badge_link_screens_status_t status);
