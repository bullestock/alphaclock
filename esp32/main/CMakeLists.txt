idf_component_register(SRCS
                       alphaclock.cpp connect.cpp console.cpp hw.cpp nvs.cpp sntp.cpp stepper.cpp
                       websocket.cpp
                       INCLUDE_DIRS "."
                       REQUIRES console esp_app_format esp_driver_gpio esp_driver_gptimer
                       esp_driver_uart esp_http_server esp_timer esp_wifi nvs_flash)
