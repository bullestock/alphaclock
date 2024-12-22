#pragma once

#include "defs.h"

void init_nvs();

wifi_creds_t get_wifi_creds();

void clear_wifi_credentials();
void add_wifi_credentials(const char* ssid, const char* password);

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:

