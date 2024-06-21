#pragma once

#include <stdint.h>
#ifndef CATDOS_MODULE_H
  #define CATDOS_MODULE_H

void catdos_module_begin(void);

void catdos_module_set_config(char* ssid, char* passwd);
#endif  // CATDOS_MODULE_H
