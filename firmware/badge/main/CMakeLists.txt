set(modules "modules/keyboard/keyboard_module.c"
            "modules/menu_screens/menu_screens_modules.c"
            "modules/oled_screen/oled_screen.c"
            "modules/wifi/wifi_module.c"
            "modules/wifi/wifi_screens_module.c"
            "modules/zigbee/zigbee_module.c"
            "modules/zigbee/zigbee_screens_module.c"
            "modules/ble/ble_module.c"
            "modules/ble/ble_screens_module.c"
            "modules/open_thread/open_thread_module.c"
            "modules/open_thread/open_thread_screens_module.c"
            "modules/games/games_screens_module.c"
            "modules/games/rope_game.c"
            "modules/games/speed_bag/speed_bag_game.c"
            "modules/games/arm_wrestling_game.c"
            "modules/games/games_module.c"
            "modules/bsides_ajolote/ajo_module.c"
            "modules/cat_dos/catdos_module.c"
            "modules/cat_dos/cat_console.c"
            "modules/cat_dos/cmd_catdos.c"
            "modules/badge_link/badge_link_module.c"
            "modules/badge_link/badge_link_screens_module.c"
            "modules/neopixels/neopixels_module.c"
            "modules/neopixels/neopixels_events.c")

set(drivers "drivers/oled_driver/oled_driver.c"
            "drivers/oled_driver/oled_driver_i2c.c"
            "drivers/oled_driver/oled_driver_spi.c")

idf_component_register(
                    SRCS
                    "main.c"
                    "${modules}"
                    "${drivers}"
                    INCLUDE_DIRS
                    "."
                    "modules/keyboard/"
                    "modules/menu_screens/"
                    "drivers/oled_driver/"
                    "modules/oled_screen/"
                    "modules/wifi/"
                    "modules/ble/"
                    "modules/zigbee/"
                    "modules/open_thread/"
                    "modules/games/"
                    "modules/bsides_ajolote/"
                    "modules/cat_dos/"
                    "modules/badge_link/"
                    "modules/games/speed_bag/"
                    "modules/neopixels/")

# # Append all directories to INCLUDE_DIRS

# file(GLOB_RECURSE ALL_INCLUDE_PATHS "./*")
# set(INCLUDE_DIRS "")

# foreach(path ${ALL_INCLUDE_PATHS})
#     get_filename_component(parentDir ${path} DIRECTORY)

#     if(IS_DIRECTORY ${parentDir})
#         list(APPEND INCLUDE_DIRS ${parentDir})
#     endif()
# endforeach()

# list(REMOVE_DUPLICATES INCLUDE_DIRS)

# # Append all source files to SRCS

# file(GLOB_RECURSE ALL_SOURCE_FILES "./*")
# set(SRCS "")

# foreach(file ${ALL_SOURCE_FILES})
#     get_filename_component(extension ${file} EXT)
#     if(NOT extension STREQUAL "")
#         list(APPEND SRCS ${file})
#     endif()
# endforeach()

# # Register the component

# idf_component_register(
#                     SRCS
#                     ${SRCS}
#                     INCLUDE_DIRS
#                     ${INCLUDE_DIRS})
