#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/timers.h"

#include "esp_task_wdt.h"


esp_err_t event_handler(void *ctx, system_event_t *event)
{
    if (event->event_id == SYSTEM_EVENT_SCAN_DONE)
    {
        printf("Number of access points found: %d\n",
               event->event_info.scan_done.number);
        uint16_t apCount = event->event_info.scan_done.number;
        if (apCount == 0)
        {
            return ESP_OK;
        }
        wifi_ap_record_t *list =
            (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
        int i;
        for (i = 0; i < apCount; i++)
        {
            char *authmode;
            switch (list[i].authmode)
            {
            case WIFI_AUTH_OPEN:
                authmode = "WIFI_AUTH_OPEN";
                break;
            case WIFI_AUTH_WEP:
                authmode = "WIFI_AUTH_WEP";
                break;
            case WIFI_AUTH_WPA_PSK:
                authmode = "WIFI_AUTH_WPA_PSK";
                break;
            case WIFI_AUTH_WPA2_PSK:
                authmode = "WIFI_AUTH_WPA2_PSK";
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                break;
            default:
                authmode = "Unknown";
                break;
            }
            printf("ssid=%s, rssi=%d, authmode=%s\n",
                   list[i].ssid, list[i].rssi, authmode);
        }
        free(list);
    }
    return ESP_OK;
}

#define TAG "TIME"

/* timer calls the function ping after interval time. xTimerCreate() takes interval in TICKs so
pdMS_TO_TICKS() converts ms interval to appropriate TICKS. pdTRUE will set timer to call periodically and if Set pdFALSE,
function is called once only */
TimerHandle_t tmr;
int id = 1;
int interval = 1000;
int currentOP = 0;
void timerCallBack(TimerHandle_t xTimer)
{
    //esp_task_wdt_reset();
    //ESP_LOGI(TAG, "tring tring!!!");
    printf("Timer CALLBACK\n");
    gpio_set_level(GPIO_NUM_2, !currentOP);
    currentOP = !currentOP;
}

int app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Let us test a WiFi scan ...
    wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = 1};
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, 0));

    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    tmr = xTimerCreate("MyTimer", pdMS_TO_TICKS(interval), pdTRUE, (void *)id, &timerCallBack);
    if (xTimerStart(tmr, 10) != pdPASS)
    {
        printf("Timer start error");
    }

    while (1)
    {
             printf("loop\n");
        //     gpio_set_level(GPIO_NUM_2, 0);
        //     currentOP = 0;
        //     vTaskDelay(4000 / portTICK_RATE_MS);
        //     printf("On\n");
        //     gpio_set_level(GPIO_NUM_2, 1);
        //     currentOP = 1;
             vTaskDelay(5000 / portTICK_RATE_MS);
    }

    return 0;
}