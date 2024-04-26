/* M5stack Task

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

#include "ili9340.h"
#include "fontx.h"
#include "cmd.h"
#include "mpc.h"

// M5Stack stuff
#define SCREEN_WIDTH	320
#define SCREEN_HEIGHT	240
#define CS_GPIO			14
#define DC_GPIO			27
#define RESET_GPIO		33
#define BL_GPIO			32
#define DISPLAY_LENGTH	26
#define GPIO_INPUT_A	GPIO_NUM_39
#define GPIO_INPUT_B	GPIO_NUM_38
#define GPIO_INPUT_C	GPIO_NUM_37

extern QueueHandle_t xQueueCmd;
extern QueueHandle_t xQueueRequest;
extern QueueHandle_t xQueueResponse;

// Left Button Monitoring
void buttonA(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	REQUEST_t requestBuf;
	requestBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_A);
	gpio_set_direction(GPIO_INPUT_A, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_A);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_A);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			strcpy(requestBuf.command, "pause");
			if (diffTick > 200) strcpy(requestBuf.command, "play");
			ESP_LOGI(pcTaskGetName(0), "requestBuf.command=[%s]", requestBuf.command);
			xQueueSend(xQueueRequest, &requestBuf, 0);
			strcpy(requestBuf.command, "status");
			xQueueSend(xQueueRequest, &requestBuf, 0);
		}
		vTaskDelay(1);
	}
}

// Middle Button Monitoring
void buttonB(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	REQUEST_t requestBuf;
	requestBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_B);
	gpio_set_direction(GPIO_INPUT_B, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_B);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_B);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			strcpy(requestBuf.command, "next");
			if (diffTick > 200) strcpy(requestBuf.command, "previous");
			xQueueSend(xQueueRequest, &requestBuf, 0);
			strcpy(requestBuf.command, "currentsong");
			xQueueSend(xQueueRequest, &requestBuf, 0);
		}
		vTaskDelay(1);
	}
}

// Right Button Monitoring
void buttonC(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");
	REQUEST_t requestBuf;
	requestBuf.taskHandle = xTaskGetCurrentTaskHandle();

	// set the GPIO as a input
	gpio_reset_pin(GPIO_INPUT_C);
	gpio_set_direction(GPIO_INPUT_C, GPIO_MODE_DEF_INPUT);

	while(1) {
		int level = gpio_get_level(GPIO_INPUT_C);
		if (level == 0) {
			ESP_LOGI(pcTaskGetName(0), "Push Button");
			TickType_t startTick = xTaskGetTickCount();
			while(1) {
				level = gpio_get_level(GPIO_INPUT_C);
				if (level == 1) break;
				vTaskDelay(1);
			}
			TickType_t endTick = xTaskGetTickCount();
			TickType_t diffTick = endTick-startTick;
			strcpy(requestBuf.command, "volume -5");
			if (diffTick > 200) strcpy(requestBuf.command, "volume +5");
			xQueueSend(xQueueRequest, &requestBuf, 0);
			strcpy(requestBuf.command, "status");
			xQueueSend(xQueueRequest, &requestBuf, 0);
		}
		vTaskDelay(1);
	}
}

int16_t getItem(char * payload, char * key, char * value, size_t value_len) {
	char *pos1;
	char *pos2;
	ESP_LOGD("getItem", "key=[%s]", key);
	pos1 = strstr(payload, key);
	ESP_LOGD("getItem", "pos1=%p", pos1);
	if (pos1 == NULL) return 0;

	char crlf[2];
	crlf[0] = 0x0a;
	crlf[1] = 0x00;
	pos1 = pos1 + strlen(key);;
	pos2 = strstr(pos1, crlf);
	ESP_LOGD("getItem", "pos2=%p", pos2);
	if (pos2 == NULL) return 0;

	int len = pos2 - pos1;
	ESP_LOGD("getItem", "len=%d", len);
	if (len > value_len) len = value_len;
	strncpy(value, pos1, len);
	value[len] = 0;
	return(strlen(value));
}

void setChar(char * target, size_t targetLen, char * source, size_t sourceLen) {
	if (targetLen > sourceLen) {
		strcpy(target, source);
	} else {
		strncpy(target, source, targetLen);
	}

}

void getStatus(char *payload, STATUS_t *status)
{
	char value[64];
	int16_t value_len;
	value_len = getItem(payload, "volume: ", value, sizeof(value)-1);
	ESP_LOGD("getStatus", "key=[volume] value=[%s] value_len=%d", value, value_len);
	status->volume = atoi(value);

	value_len = getItem(payload, "state: ", value, sizeof(value)-1);
	ESP_LOGD("getStatus", "key=[state] value=[%s] value_len=%d", value, value_len);
	setChar(status->state, sizeof(status->state), value, value_len);

	ESP_LOGD("getStatus", "status->volume=%d", status->volume);
	ESP_LOGD("getStatus", "status->state=%s", status->state);
}

void getCurrentsong(char *payload, CURRENTSONG_t *currentsong)
{
	char value[64];
	int16_t value_len;
	value_len = getItem(payload, "Artist: ", value, sizeof(value)-1);
	ESP_LOGD("getCurrentsong", "key=[Artist] value=[%s] value_len=%d", value, value_len);
	setChar(currentsong->Artist, sizeof(currentsong->Artist), value, value_len);

	value_len = getItem(payload, "Title: ", value, sizeof(value)-1);
	ESP_LOGD("getCurrentsong", "key=[Title] value=[%s] value_len=%d", value, value_len);
	setChar(currentsong->Title, sizeof(currentsong->Title), value, value_len);

	value_len = getItem(payload, "Album: ", value, sizeof(value)-1);
	ESP_LOGD("getCurrentsong", "key=[Album] value=[%s] value_len=%d", value, value_len);
	setChar(currentsong->Album, sizeof(currentsong->Album), value, value_len);

	value_len = getItem(payload, "Track: ", value, sizeof(value)-1);
	ESP_LOGD("getCurrentsong", "key=[Track] value=[%s] value_len=%d", value, value_len);
	currentsong->Track = atoi(value);

	ESP_LOGD("getCurrentsong", "currentsong->Artist=%s", currentsong->Artist);
	ESP_LOGD("getCurrentsong", "currentsong->Title=%s", currentsong->Title);
	ESP_LOGD("getCurrentsong", "currentsong->Album=%s", currentsong->Album);
	ESP_LOGD("getCurrentsong", "currentsong->Track=%d", currentsong->Track);
}

void tft(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(0), "Start");

	// Set font file
	FontxFile fx[2];
#if CONFIG_ESP_FONT_GOTHIC
	InitFontx(fx,"/fonts/ILGH24XB.FNT",""); // 12x24Dot Gothic
#endif
#if CONFIG_ESP_FONT_MINCYO
	InitFontx(fx,"/fonts/ILMH24XB.FNT",""); // 12x24Dot Mincyo
#endif

	// Get font width & height
	uint8_t buffer[FontxGlyphBufSize];
	uint8_t fontWidth;
	uint8_t fontHeight;
	GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
	ESP_LOGI(pcTaskGetName(0), "fontWidth=%d fontHeight=%d",fontWidth,fontHeight);
	size_t maxChar = SCREEN_WIDTH / fontWidth;
	ESP_LOGI(pcTaskGetName(0), "maxChar=%d",maxChar);

	// Request play , status , currentsong
	REQUEST_t requestBuf;
	requestBuf.taskHandle = xTaskGetCurrentTaskHandle();
	strcpy(requestBuf.command, "playlist");
	xQueueSend(xQueueRequest, &requestBuf, 0);
	strcpy(requestBuf.command, "status");
	xQueueSend(xQueueRequest, &requestBuf, 0);
	strcpy(requestBuf.command, "currentsong");
	xQueueSend(xQueueRequest, &requestBuf, 0);

	// Setup Screen
	TFT_t dev;
	spi_master_init(&dev, CS_GPIO, DC_GPIO, RESET_GPIO, BL_GPIO);
	lcdInit(&dev, 0x9341, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	ESP_LOGI(pcTaskGetName(0), "Setup Screen done");

	int lines = (SCREEN_HEIGHT - fontHeight) / fontHeight;
	ESP_LOGD(pcTaskGetName(0), "SCREEN_HEIGHT=%d fontHeight=%d lines=%d", SCREEN_HEIGHT, fontHeight, lines);

	// Clear Screen
	lcdFillScreen(&dev, BLACK);
	lcdSetFontDirection(&dev, 0);

	// Reset scroll area
	lcdSetScrollArea(&dev, 0, 0x0140, 0);

	// Show header
	uint8_t ascii[44];
	uint16_t ypos = fontHeight-1;
	strcpy((char *)ascii, "MPD Client");
	lcdDrawString(&dev, fx, 0, ypos, ascii, YELLOW);
	uint16_t xstatus = 11*fontWidth;

	RESPONSE_t responseBuf;
	STATUS_t status;
	STATUS_t statusOld;
	bzero(statusOld.state, sizeof(statusOld.state));
	CURRENTSONG_t currentsong;
	CURRENTSONG_t currentsongOld;
	bzero(currentsongOld.Title, sizeof(currentsongOld.Title));
	bool existPlaylist = false;

	while(1) {
		xQueueReceive(xQueueResponse, &responseBuf, portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(0),"command=%s length=%d existPlaylist=%d", 
				responseBuf.command, strlen(responseBuf.payload), existPlaylist);
		ESP_LOGD(pcTaskGetName(0),"\n%s", responseBuf.payload);
		if (strcmp(responseBuf.command, "playlist") == 0) {
			if (strlen(responseBuf.payload) == 0) {
				uint16_t ypos = fontHeight*3-1;
				lcdDrawFillRect(&dev, 0, ypos-fontHeight, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
				bzero(ascii, sizeof(ascii));
				strcpy((char *)ascii, "There is No Playlist.");
				lcdDrawString(&dev, fx, 0, ypos, ascii, RED);
				ypos = ypos + fontHeight;
				bzero(ascii, sizeof(ascii));
				strcpy((char *)ascii, "MPC load <file>");
				lcdDrawString(&dev, fx, 0, ypos, ascii, RED);
				existPlaylist = false;
			} else {
				existPlaylist = true;
			}
		}

		if (strcmp(responseBuf.command, "status") == 0) {
			if (! existPlaylist) continue;
			getStatus(responseBuf.payload, &status);
			if (strcmp(statusOld.state, status.state) == 0 && statusOld.volume == status.volume) continue;
		
			lcdDrawFillRect(&dev, xstatus, 0, SCREEN_WIDTH-1, fontHeight-1, BLACK);
			strcpy((char *)ascii, status.state);
			if (strcmp(status.state, "play") == 0) {
				lcdDrawString(&dev, fx, xstatus, fontHeight-1, ascii, GREEN);
				sprintf((char *)ascii, "volume:%d", status.volume);
				lcdDrawString(&dev, fx, xstatus+(fontWidth*6), fontHeight-1, ascii, GREEN);
			} else {
				lcdDrawString(&dev, fx, xstatus, fontHeight-1, ascii, RED);
			}
			statusOld = status;
		}

		if (strcmp(responseBuf.command, "currentsong") == 0) {
			if (! existPlaylist) continue;

			if (strlen(responseBuf.payload) == 0) {
				// Request play , status , currentsong
				strcpy(requestBuf.command, "play");
				xQueueSend(xQueueRequest, &requestBuf, 0);
				strcpy(requestBuf.command, "status");
				xQueueSend(xQueueRequest, &requestBuf, 0);
				strcpy(requestBuf.command, "currentsong");
				xQueueSend(xQueueRequest, &requestBuf, 0);
				continue;
			}

			ESP_LOGD(pcTaskGetName(0),"\n%s", responseBuf.payload);
			getCurrentsong(responseBuf.payload, &currentsong);
			if (strcmp(currentsongOld.Title, currentsong.Title) == 0) continue;

			uint16_t ypos = fontHeight*3-1;
			lcdDrawFillRect(&dev, 0, ypos-fontHeight, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, BLACK);
			bzero(ascii, sizeof(ascii));
			strncpy((char *)ascii, currentsong.Title, maxChar);
			lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			ypos = ypos + fontHeight;
			if (strlen(currentsong.Title) > maxChar) {
				bzero(ascii, sizeof(ascii));
				strncpy((char *)ascii, &currentsong.Title[maxChar], maxChar);
				lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			}

			ypos = ypos + fontHeight;
			bzero(ascii, sizeof(ascii));
			strncpy((char *)ascii, currentsong.Artist, maxChar);
			lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			ypos = ypos + fontHeight;
			if (strlen(currentsong.Artist) > maxChar) {
				bzero(ascii, sizeof(ascii));
				strncpy((char *)ascii, &currentsong.Artist[maxChar], maxChar);
				lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			}

			ypos = ypos + fontHeight;
			bzero(ascii, sizeof(ascii));
			strncpy((char *)ascii, currentsong.Album, maxChar);
			lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			ypos = ypos + fontHeight;
			if (strlen(currentsong.Album) > maxChar) {
				bzero(ascii, sizeof(ascii));
				strncpy((char *)ascii, &currentsong.Album[maxChar], maxChar);
				lcdDrawString(&dev, fx, 0, ypos, ascii, CYAN);
			}
			currentsongOld = currentsong;
		}
	}

	// nerver reach here
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
