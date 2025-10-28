# ESP-IDF examples

## VS Code Extension
https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/

## esp http client example code
```c
#include <esp_http_client.h>
#include <esp_log.h>

static const char *TAG = "HTTP_CLIENT";

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s\n", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        // ... other event handling
        default:
            break;
    }
    return ESP_OK;
}

void http_client_get_example(void) {
    esp_http_client_config_t config = {
        .url = "http://example.com/api/data",
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}
'''
