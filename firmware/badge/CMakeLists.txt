# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS components/oled_driver)
set(EXTRA_COMPONENT_DIRS components/wifi_sniffer)
set(EXTRA_COMPONENT_DIRS components/buzzer)
set(EXTRA_COMPONENT_DIRS components/iperf)
set(EXTRA_COMPONENT_DIRS components/bluetooth_scanner)
set(EXTRA_COMPONENT_DIRS components/leds)
set(EXTRA_COMPONENT_DIRS components/sd_card)
set(EXTRA_COMPONENT_DIRS components/preferences)
set(EXTRA_COMPONENT_DIRS components/ieee802154)
set(EXTRA_COMPONENT_DIRS components/openthread)
set(EXTRA_COMPONENT_DIRS components/wifi_captive)
set(EXTRA_COMPONENT_DIRS components/radio_selector)
set(EXTRA_COMPONENT_DIRS components/cmd_wifi)
set(EXTRA_COMPONENT_DIRS $ENV{BADGE_CONNECT_SRC_PATH}) # Set this path when you want to use badge_connect_src, see firmware/badge/README.md for details

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(badge)
