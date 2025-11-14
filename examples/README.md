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
```

### RGB Code Parsing
```C
#include <stdio.h>
int get_color(char* color_code, int* rgb)
{
    int first_digit, second_digit;
    
    for(int i=0; i<3; ++i){
        
        if (color_code[i*2 + 0] >= '0' && color_code[i*2 + 0] <= '9') {
            first_digit = color_code[i*2 + 0] - '0';
        } else {
            first_digit = color_code[i*2 + 0] - 'A' + 10;
        }
        if (color_code[i*2 + 1] >= '0' && color_code[i*2 + 1] <= '9') {
            second_digit = color_code[i*2 + 1] - '0';
        } else {
            second_digit = color_code[1] - 'A' + 10;
        }
        rgb[i] = first_digit * 16 + second_digit;
    }
    
    return 0;
}


int main()
{
    char color[] = "FFFFFF";
    int rgb[3];
    get_color(color, rgb);
    printf("RGB: (%d, %d, %d)", rgb[0], rgb[1], rgb[2]);

    return 0;
}
```
## I2S Setup
### INMP 411 MEMS Microphone
<img width="220" height="220" alt="image" src="https://github.com/user-attachments/assets/2525c583-1d49-41f4-9df1-a2eacb9be9da" />

### ESP32-S3 DevKit
<img width="1584" height="1080" alt="image" src="https://github.com/user-attachments/assets/dfbf904f-a779-4cec-9496-7c9d0529dc42" />

### GPIO PIN Setup
```C
#elif CONFIG_IDF_TARGET_ESP32S3
#define EXAMPLE_I2S_BCLK_IO1        GPIO_NUM_2      // I2S bit clock io number
#define EXAMPLE_I2S_WS_IO1          GPIO_NUM_3      // I2S word select io number
#define EXAMPLE_I2S_DOUT_IO1        GPIO_NUM_4      // I2S data out io number
#define EXAMPLE_I2S_DIN_IO1         GPIO_NUM_5      // I2S data in io number
// PDM RX 4 line IO
#define EXAMPLE_I2S_DIN1_IO1        GPIO_NUM_6     // I2S data in io number
#define EXAMPLE_I2S_DIN2_IO1        GPIO_NUM_7     // I2S data in io number
#define EXAMPLE_I2S_DIN3_IO1        GPIO_NUM_8     // I2S data in io number
// STD/TDM simplex pins
#define EXAMPLE_I2S_BCLK_IO2        GPIO_NUM_6      // I2S bit clock io number
#define EXAMPLE_I2S_WS_IO2          GPIO_NUM_7      // I2S word select io number
#define EXAMPLE_I2S_DOUT_IO2        GPIO_NUM_8      // I2S data out io number
#define EXAMPLE_I2S_DIN_IO2         GPIO_NUM_9      // I2S data in io number
```
