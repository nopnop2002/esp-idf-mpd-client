/* Encorder Task

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "cmd.h"
#include "encoder.h"

#if CONFIG_ENCODER
extern QueueHandle_t xQueueCmd;
extern QueueHandle_t xQueueRequest;

#define CONFIG_RE_A_GPIO	21
#define CONFIG_RE_B_GPIO	22
#define CONFIG_RE_BTN_GPIO	99

#define GPIO_BIT(x) (1ULL << (x))

static const int8_t valid_states[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };


void encoder(void *pvParameters)
{
	ESP_LOGI(pcTaskGetTaskName(0), "Start");
	ESP_LOGI(pcTaskGetTaskName(0), "RE_A_GPIO=%d RE_B_GPIO=%d", CONFIG_RE_A_GPIO, CONFIG_RE_B_GPIO);
	ESP_LOGI(pcTaskGetTaskName(0), "GPIO_NUM_MAX=%d", GPIO_NUM_MAX);
	rotary_encoder_t re;
	re.pin_a = CONFIG_RE_A_GPIO;
	re.pin_b = CONFIG_RE_B_GPIO;
	re.pin_btn = CONFIG_RE_BTN_GPIO;

	REQUEST_t requestBuf;
	requestBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// setup GPIO
	gpio_config_t io_conf;
	memset(&io_conf, 0, sizeof(gpio_config_t));
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = GPIO_BIT(re.pin_a) | GPIO_BIT(re.pin_b);
	if (re.pin_btn < GPIO_NUM_MAX)
		io_conf.pin_bit_mask |= GPIO_BIT(re.pin_btn);
	ESP_ERROR_CHECK(gpio_config(&io_conf));

	re.btn_state = RE_BTN_RELEASED;
	re.btn_pressed_time_us = 0;


	while(1) {
		re.code <<= 2;
		re.code |= gpio_get_level(re.pin_a);
		re.code |= gpio_get_level(re.pin_b) << 1;
		re.code &= 0xf;

		if (valid_states[re.code]) {
			int8_t inc = 0;

			re.store = (re.store << 4) | re.code;
			ESP_LOGI(pcTaskGetTaskName(0), "re.code=%x re.store=%x", re.code, re.store);

			if (re.store == 0xe817) inc = 1;
			if (re.store == 0xd42b) inc = -1;

			if (inc)
			{
				ESP_LOGI(pcTaskGetTaskName(0), "inc=%d", inc);
				strcpy(requestBuf.command, "volume -1");
				if (inc > 0) strcpy(requestBuf.command, "volume +1");
				xQueueSend(xQueueRequest, &requestBuf, 0);
				strcpy(requestBuf.command, "status");
				xQueueSend(xQueueRequest, &requestBuf, 0);
			}
		}
		vTaskDelay(1);
	} // end while
}
#endif
