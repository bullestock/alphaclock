
#pragma once

#include <esp_http_server.h>

httpd_handle_t start_webserver();

extern HourMode active_hour_mode;
