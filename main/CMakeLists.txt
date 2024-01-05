idf_component_register(SRCS "main.c"
                            "wifi_manager.c"
                            "websocket_manager.c"
                            "camera_manager.c"
                    INCLUDE_DIRS "."
                                 "../components/esp32_camera_driver/driver/"
                                 "../components/esp32_camera_driver/driver/private_include"
                                 "../components/ai_model/")
