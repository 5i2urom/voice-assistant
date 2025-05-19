idf_component_register(SRCS "main.c" "i2c-lcd.c" "ble-connect.c" "wifi-connect.c" "server-config.c" "network-monitor.c"                        
                    INCLUDE_DIRS "." "../src")

target_compile_options(${COMPONENT_LIB} PRIVATE -O3 -ffast-math)

                    
