set(srcs "app2_main.c" "eeprom.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
