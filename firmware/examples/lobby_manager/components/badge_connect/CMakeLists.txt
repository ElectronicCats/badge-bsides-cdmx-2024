idf_component_register(SRCS "badge_connect.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi
                    PRIV_REQUIRES nvs_flash espressif__esp-now)
