idf_component_register(SRCS "main.c" "i2c-lcd.c" "ble-connect.c" "wifi-connect.c" "server-config.c" "network-monitor.c" "../src/wake_word_detector.cpp" "../src/state_machine/DetectWakeWordState.cpp"
                    INCLUDE_DIRS "." "../src" "../src/state_machine")

# idf_component_register(SRCS "main.c"
#                        INCLUDE_DIRS ".")

# target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -ffast-math)

                    
