idf_component_register(SRCS "main.c"
                            "wifi_manager.c"
                            "http_post_manager.c"
                            "camera_manager.c"
                    INCLUDE_DIRS "."
                                 "../components/esp32_camera/driver/"
                                 "../components/esp32_camera/driver/private_include")
