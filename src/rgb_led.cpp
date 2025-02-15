#include "rgb_led.h"

static rmt_data_t led_data[24];

static float rgb_period = 3000.0;

const int rgb_sine_len = 256;
static uint8_t rgb_sine[rgb_sine_len];
static bool    rgb_sine_is_init = false;

static TaskHandle_t rgb_task_handle = NULL;

static void IRAM_ATTR __attribute__((optimize("-O3"))) rmt_write_bitfield(const uint32_t bitfield, const bool async=true)
{
    int i = 0, bit;

    // rmt low / high
    static const uint32_t DRAM_ATTR rmt_data[2] = {RMT_DATA(4, 1, 8, 0), RMT_DATA(8, 1, 4, 0)};
    static rmt_data_t led_data[24];

    for (bit = 23; bit >= 0; bit--)
    {
        bool bit_state = !!(bitfield & BIT(bit));
        led_data[i++].val = rmt_data[bit_state];
    }
    if (likely(async))
        rmtWriteAsync(48, led_data, 24);
    else rmtWrite(48, led_data, 24, 100);
}

static inline void IRAM_ATTR rmt_write_to_led(const uint8_t red, const uint8_t green, const uint8_t blue, const bool async=true)
{
    rmt_write_bitfield(RGB24BIT(red, green, blue), async);
}

float rgb_get_period()
{
    if (rgb_task_handle == NULL)
        return NAN;
    return rgb_period;
}

bool rgb_get_state() { return rgb_task_handle != NULL; }

esp_err_t rgb_set_period(const float f, const TickType_t xTicksToWait)
{
    if (rgb_task_handle == NULL)
        return ESP_FAIL;
    
    uint32_t val = *(uint32_t*)&f;
    return xTaskNotify(rgb_task_handle, val, eSetValueWithOverwrite) == pdTRUE ? ESP_OK : ESP_FAIL;
}

/**
 * @brief Initialize RMT module
 *
 * @return esp_err_t
 */
static esp_err_t rgb_init_rmt()
{
    if (!rmtInit(48, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000))
        return ESP_FAIL;
    return ESP_OK;
}

static void __attribute__((optimize("-O3"))) 
init_sin_table(uint8_t sine_table[], const int length, int min, int max) 
{
    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

    const float amplitude = max - min;
    const int length_div_2 = length / 2;

    for (int i = 0; i < length; i++)
    {
        float sin_pos = (sinf(i * PI / length_div_2) + 1) / 2;
        sine_table[i] = roundf(sin_pos * amplitude + min);
    }
}

void rgb_task(void *param)
{
    // initialize sine table and offset
    Serial.println("TESTTESTTESTTESTTESTTEST");
    if (!rgb_sine_is_init) {
        init_sin_table(rgb_sine, rgb_sine_len, 0, 255);
        rgb_sine_is_init = true;
    }

    // led color offset
    uint8_t offsets[3] = {
        0,
        (rgb_sine_len * 2 + 1) / 3,
        (rgb_sine_len + 1) / 3,
    };

    // init RMT
    ESP_ERROR_CHECK(rgb_init_rmt());

    // turn off LED
    rmt_write_to_led(0, 0, 0);

    // led delay
    int delay_by = 0;
    float a = 0;

    int64_t delta = 0;
    int64_t time_guide = esp_timer_get_time() / 1000;

    while (true)
    {
        // correction timer
        int64_t current_time = esp_timer_get_time() / 1000;

        // try to receive new period
        static uint32_t ul_rgb_period_tmp;
        if (xTaskNotifyWait(0, 0, &ul_rgb_period_tmp, 0) == pdTRUE)
        {
            static float rgb_period_tmp = *(float*)&ul_rgb_period_tmp;
            if (rgb_period_tmp > 50)
                rgb_period = rgb_period_tmp;
        }

        // increment offset until enough delay
        while (a < 100.f / 6)
        {
            ++offsets[0];
            ++offsets[1];
            ++offsets[2];
            a += rgb_period / rgb_sine_len;
        }
        // take integer
        delay_by = a;
        a -= delay_by;

        // get time difference between previous and current loop
        // delta indicates how much time is missed in ms
        delta += current_time - time_guide;

        // subtract the next delay with delta
        if (delay_by - delta > 10)
        {
            delay_by -= delta;
            delta = 0;
        }
        // calculate time after delay
        time_guide = current_time + delay_by;

        rmt_write_to_led(rgb_sine[offsets[0]], rgb_sine[offsets[1]],
                            rgb_sine[offsets[2]]);

        vTaskDelay(delay_by);
    }
    rmt_write_to_led(0, 0, 0, false);
    rmtDeinit(48);
    vTaskDelete(NULL);
}

esp_err_t rgb_init_task(UBaseType_t uxPriority)
{
    if (rgb_task_handle == NULL) // check if task is already created
        xTaskCreate(rgb_task, "rgb_task", 4096, NULL, uxPriority, &rgb_task_handle);
    if (rgb_task_handle == NULL) // fail to create task
        return ESP_FAIL;
    return ESP_OK;
}

esp_err_t rgb_stop_task()
{
    if (rgb_task_handle == NULL)
        return ESP_OK;

    vTaskDelete(rgb_task_handle);
    rgb_task_handle = NULL;

    rmt_write_to_led(0, 0, 0, false);
    rmtDeinit(48);

    return ESP_OK;
}


