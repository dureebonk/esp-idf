/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "sdkconfig.h"

#include <math.h>
#include "esp_dsp.h"

/* Set 1 to allocate rx & tx channels in duplex mode on a same I2S controller, they will share the BCLK and WS signal
 * Set 0 to allocate rx & tx channels in simplex mode, these two channels will be totally separated,
 * Specifically, due to the hardware limitation, the simplex rx & tx channels can't be registered on the same controllers on ESP32 and ESP32-S2,
 * and ESP32-S2 has only one I2S controller, so it can't allocate two simplex channels */
#define EXAMPLE_I2S_DUPLEX_MODE         CONFIG_USE_DUPLEX

#if CONFIG_IDF_TARGET_ESP32
#define EXAMPLE_STD_BCLK_IO1        GPIO_NUM_4      // I2S bit clock io number
#define EXAMPLE_STD_WS_IO1          GPIO_NUM_5      // I2S word select io number
#define EXAMPLE_STD_DOUT_IO1        GPIO_NUM_18     // I2S data out io number
#define EXAMPLE_STD_DIN_IO1         GPIO_NUM_19     // I2S data in io number
#if !EXAMPLE_I2S_DUPLEX_MODE
#define EXAMPLE_STD_BCLK_IO2    GPIO_NUM_22     // I2S bit clock io number
#define EXAMPLE_STD_WS_IO2      GPIO_NUM_23     // I2S word select io number
#define EXAMPLE_STD_DOUT_IO2    GPIO_NUM_25     // I2S data out io number
#define EXAMPLE_STD_DIN_IO2     GPIO_NUM_26     // I2S data in io number
#endif
#else
#define EXAMPLE_STD_BCLK_IO1        GPIO_NUM_2      // I2S bit clock io number
#define EXAMPLE_STD_WS_IO1          GPIO_NUM_3      // I2S word select io number
#define EXAMPLE_STD_DOUT_IO1        GPIO_NUM_4      // I2S data out io number
#define EXAMPLE_STD_DIN_IO1         GPIO_NUM_5      // I2S data in io number
#if !EXAMPLE_I2S_DUPLEX_MODE
#define EXAMPLE_STD_BCLK_IO2    GPIO_NUM_6      // I2S bit clock io number
#define EXAMPLE_STD_WS_IO2      GPIO_NUM_7      // I2S word select io number
#define EXAMPLE_STD_DOUT_IO2    GPIO_NUM_8      // I2S data out io number
#define EXAMPLE_STD_DIN_IO2     GPIO_NUM_9      // I2S data in io number
#endif
#endif
#define N_SAMPLES              2048
#define EXAMPLE_BUFF_SIZE               4 * N_SAMPLES   // size of buffer to read i2s data
#define TAG                             "i2s_std_example"

int N = N_SAMPLES;

// Input test array
__attribute__((aligned(16)))
float_t x1[N_SAMPLES];
// Window coefficients
__attribute__((aligned(16)))
float_t wind[N_SAMPLES];
// working complex array
__attribute__((aligned(16)))
float_t y_cf[N_SAMPLES * 2];
// Pointers to result arrays
__attribute__((aligned(16)))
float *y1_cf = &y_cf[0];

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler



static void process_and_show(float *data, int length)
{
    ESP_LOGI(TAG, "FFT processing and showing the power spectrum...");
    ESP_LOGI(TAG, "Length: %d", length);
    dsps_fft2r_fc32(data, length);
    // Bit reverse
    ESP_LOGI(TAG, "Bit reversing...");
    dsps_bit_rev_fc32(data, length);
    // Convert one complex vector to two complex vectors
    ESP_LOGI(TAG, "Cplx to Re...");
    dsps_cplx2reC_fc32(data, length);

    for (int i = 0 ; i < length / 2 ; i++) {
        data[i] = 10 * log10f((data[i * 2 + 0] * data[i * 2 + 0] + data[i * 2 + 1] * data[i * 2 + 1]) / N);
    }

    // Show power spectrum in 64x10 window from -100 to 0 dB from 0..N/4 samples
    dsps_view(data, length / 2, 64, 10,  -120, 40, '|');
}

static int get_data(const uint32_t* raw_data, size_t length)
{
    float_t abs_sum = 0.0f;
    float_t temp;
    for (int i = 0; i < length; i++) {
        // Assuming left channel only, 32-bit data
        temp = (float_t)((int32_t) raw_data[i] >> 8);
        abs_sum += fabsf(temp);
        x1[i] = temp * wind[i];
    }
    ESP_LOGI(TAG, "Input signal average amplitude: %.02f", abs_sum / length);
    return 0;
}


static void i2s_example_read_task(void *args)
{
    uint32_t *r_buf = (uint32_t *)calloc(1, EXAMPLE_BUFF_SIZE);
    assert(r_buf); // Check if r_buf allocation success
    size_t r_bytes = 0;
    size_t length = 0;

    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret  != ESP_OK) {
        ESP_LOGE(TAG, "Not possible to initialize FFT. Error = %i", ret);
        return;
    }
    dsps_wind_hann_f32(wind, N);
    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    /* ATTENTION: The print and delay in the read task only for monitoring the data by human,
     * Normally there shouldn't be any delays to ensure a short polling time,
     * Otherwise the dma buffer will overflow and lead to the data lost */
    while (1) {
        /* Read i2s data */
        if (i2s_channel_read(rx_chan, r_buf, EXAMPLE_BUFF_SIZE, &r_bytes, 1000) == ESP_OK) {
            ESP_LOGI(TAG, "*** Multiply tone signal with Hann window by esp-dsp basic math functions. ***");
            
            length = r_bytes / sizeof(uint32_t);
            get_data(r_buf, length);

            dsps_mul_f32(x1, wind, y_cf, length, 1, 1, 2); // Multiply input array with window and store as real part
            dsps_mulc_f32(&y_cf[1], &y_cf[1], length, 0, 2, 2); // Clear imaginary part of the complex signal
            process_and_show(y_cf, length);
            ESP_LOGI(TAG, "*** End Example. ***");
        } else {
            printf("Read Task: i2s read failed\n");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    free(r_buf);
    vTaskDelete(NULL);
}

#if EXAMPLE_I2S_DUPLEX_MODE
static void i2s_example_init_std_duplex(void)
{
    /* Setp 1: Determine the I2S channel configuration and allocate both channels
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    /* Step 2: Setting the configurations of standard mode, and initialize rx & tx channels
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_std.h' which can only be used in STD mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO1,
            .ws   = EXAMPLE_STD_WS_IO1,
            .dout = EXAMPLE_STD_DOUT_IO1,
            .din  = EXAMPLE_STD_DOUT_IO1, // In duplex mode, bind output and input to a same gpio can loopback internally
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* Initialize the channels */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
}

#else

static void i2s_example_init_std_simplex(void)
{
    /* Setp 1: Determine the I2S channel configuration and allocate two channels one by one
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role
     * The tx and rx channels here are registered on different I2S controller,
     * Except ESP32 and ESP32-S2, others allow to register two separate tx & rx channels on a same controller */
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    /* Step 2: Setting the configurations of standard mode and initialize each channels one by one
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_std.h' which can only be used in STD mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO1,
            .ws   = EXAMPLE_STD_WS_IO1,
            .dout = EXAMPLE_STD_DOUT_IO1,
            .din  = EXAMPLE_STD_DIN_IO1,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_STD_BCLK_IO2,
            .ws   = EXAMPLE_STD_WS_IO2,
            .dout = I2S_GPIO_UNUSED,    //EXAMPLE_STD_DOUT_IO2,
            .din  = EXAMPLE_STD_DIN_IO2,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
 };
    /* Default is only receiving left slot in mono mode,
     * update to right here to show how to change the default configuration */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std_cfg));
}
#endif

void app_main(void)
{
#if EXAMPLE_I2S_DUPLEX_MODE
    i2s_example_init_std_duplex();
#else
    i2s_example_init_std_simplex();
#endif

    /* Step 3: Create writing and reading task, enable and start the channels */
    xTaskCreate(i2s_example_read_task, "i2s_example_read_task", 4096, NULL, 5, NULL);
}
