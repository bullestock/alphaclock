#include "defs.h"
#include "websocket.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"

bool is_button_pressed = false;
int active_button = 0;
bool button_direction_up = false;
bool is_button_fast = false;

void handle_up_down_button(uint8_t arg)
{
    const bool is_up_button = arg & 0x80;
    const bool is_mouse_down = arg & 0x40;
    const bool is_fast = arg & 0x20;
    const int ident = arg & 0x03;
    if (ident > 2)
    {
        ESP_LOGE(TAG, "Invalid ident %d", ident);
        return;
    }
    static const char* identifiers = "hms";
    ESP_LOGI(TAG, "Button %c %s: %s",
             identifiers[ident],
             is_up_button ? "up" : "down",
             is_mouse_down ? "start" : "stop");
    if (is_mouse_down)
    {
        active_button = ident;
        button_direction_up = is_up_button;
        is_button_fast = is_fast;
        is_button_pressed = true;
    }
    else
        is_button_pressed = false;
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    uint8_t* buf = nullptr;
    // Set max_len = 0 to get the frame len
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    if (ws_pkt.len)
    {
        buf = reinterpret_cast<uint8_t*>(calloc(1, ws_pkt.len + 1));
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        // Set max_len = ws_pkt.len to get the frame payload
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        if (ws_pkt.len < 2)
        {
            ESP_LOGE(TAG, "WS frame too short: %d", (int) ws_pkt.len);
            free(buf);
            return ESP_ERR_INVALID_ARG;
        }
        if (ws_pkt.type != HTTPD_WS_TYPE_BINARY)
        {
            ESP_LOGE(TAG, "WS frame not binary: %d", (int) ws_pkt.type);
            free(buf);
            return ESP_ERR_INVALID_ARG;
        }
        ESP_LOGI(TAG, "Got WS message: %02X %02X",
                 ws_pkt.payload[0], ws_pkt.payload[1]);
        switch (ws_pkt.payload[0])
        {
        case 0:
            handle_up_down_button(ws_pkt.payload[1]);
            break;

        default:
            ESP_LOGE(TAG, "Invalid WS command: %02X", ws_pkt.payload[0]);
            break;
        }
    }
    /*
    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK)
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    */
    free(buf);
    return ret;
}

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
const size_t index_html_size = index_html_end - index_html_start;

extern const uint8_t index_js_start[] asm("_binary_index_js_start");
extern const uint8_t index_js_end[] asm("_binary_index_js_end");
const size_t index_js_size = index_js_end - index_js_start;

static esp_err_t root_get_handler(httpd_req_t* req)
{
    ESP_LOGI(TAG, "URI: %s", req->uri);
    if (strlen(req->uri) == 1)
    {
        httpd_resp_send_chunk(req, reinterpret_cast<const char*>(index_html_start), index_html_size);
    }
    else if (!strcmp(req->uri, "/index.js"))
    {
        httpd_resp_set_type(req, "text/javascript");
        httpd_resp_send_chunk(req, reinterpret_cast<const char*>(index_js_start), index_js_size);
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t ws = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = ws_handler,
    .user_ctx   = nullptr,
    .is_websocket = true
};

static const httpd_uri_t root = {
    .uri       = "/*",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = nullptr,
    .is_websocket = false
};


httpd_handle_t start_webserver()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
