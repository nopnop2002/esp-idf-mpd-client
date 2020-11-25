/* MPC Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "cmd.h"

#define PORT 6600

static const char *TAG = "TCP";

extern QueueHandle_t xQueueRequest;
extern QueueHandle_t xQueueResponse;

void tcp_client_task(void *pvParameters)
{
	char tx_buffer[128];
	char rx_buffer[128];
	char end_buffer[5];
	char host_ip[] = CONFIG_ESP_MPD_IPV4_ADDR;
	int addr_family = 0;
	int ip_protocol = 0;
	//end_buffer[0] = 0x0a; // LF
	end_buffer[0] = 0x4f; // O
	end_buffer[1] = 0x4b; // K
	end_buffer[2] = 0x0a; // LF
	end_buffer[3] = 0x00; 

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(host_ip);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	int sock =	socket(addr_family, SOCK_STREAM, ip_protocol);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
	}
	ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

	int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
		vTaskDelete(NULL);
	}
	ESP_LOGI(TAG, "Successfully connected");

	// Receive version message 
	while (1) {
		int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
		// Error occurred during receiving
		if (len < 0) {
			ESP_LOGE(TAG, "recv failed: errno %d", errno);
			vTaskDelete(NULL);
		}
		// Data received
		else {
			rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
			ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
			ESP_LOGI(TAG, "[%s]", rx_buffer);
			if (strncmp(rx_buffer, "OK MPD", 6) == 0) break;
		}
	}
	ESP_LOGI(TAG, "Connected to MPD");

	REQUEST_t requestBuf;
	RESPONSE_t responseBuf;

	while (1) {
		ESP_LOGI(TAG, "Waitting request");
		xQueueReceive(xQueueRequest, &requestBuf, portMAX_DELAY);
		ESP_LOGI(TAG,"requestBuf.command=%s", requestBuf.command);
		//strcpy(tx_buffer, "listplaylists\n");
		//strcpy(tx_buffer, "status\n");
		strcpy(tx_buffer, requestBuf.command);
		strcat(tx_buffer, "\n");
		bool response = false;
		if (strcmp(requestBuf.command, "playlist") == 0) response = true;
		if (strcmp(requestBuf.command, "status") == 0) response = true;
		if (strcmp(requestBuf.command, "currentsong") == 0) response = true;
		if (response) {
			strcpy(responseBuf.command, requestBuf.command);
			memset(responseBuf.payload, 0, sizeof(responseBuf.payload));
		}

		err = send(sock, tx_buffer, strlen(tx_buffer), 0);
		if (err < 0) {
			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
			break;
		}

		struct timeval receiving_timeout;
		receiving_timeout.tv_sec = 5;
		receiving_timeout.tv_usec = 0;
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
				sizeof(receiving_timeout)) < 0) {
			ESP_LOGE(TAG, "... failed to set socket receiving timeout");
			break;
		}
		ESP_LOGD(TAG, "... set socket receiving timeout success");

		size_t payloadLength = 0;
		while (1) {
			int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
			// Error occurred during receiving
			if (len < 0) {
				if (errno == EAGAIN) break; // Timeout
				ESP_LOGE(TAG, "recv failed: errno %d", errno);
				break;
			}
			// Data received
			else {
				rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
				ESP_LOGD(TAG, "Received %d bytes from %s:", len, host_ip);
				ESP_LOGD(TAG, "[%s]", rx_buffer);
				ESP_LOG_BUFFER_HEXDUMP(TAG, rx_buffer, len, ESP_LOG_DEBUG);
				payloadLength = payloadLength + len;
				if (response) {
					if (payloadLength < sizeof(responseBuf.payload)) {
						strcat(responseBuf.payload, rx_buffer);
					}
				}

				/* 
				This statement is incomplete.
				The letter O and the letter K will be received in separate packets.
				If the letter O and the letter K are split into separate packets, 
				the receive timeout will occur.
				*/
				if (strstr(rx_buffer, end_buffer) != NULL) break;
			}
		}
		ESP_LOGI(TAG, "reading end. payloadLength=%d response=%d", payloadLength, response);
		if (response) {
			ESP_LOGD(TAG, "\n%s", responseBuf.payload);
			ESP_LOG_BUFFER_HEXDUMP(TAG, responseBuf.payload, payloadLength, ESP_LOG_DEBUG);
			if (strcmp(responseBuf.payload, end_buffer) == 0) {
				responseBuf.payload[0] = 0;
				xQueueSend(xQueueResponse, &responseBuf, 0);
			} else {
				xQueueSend(xQueueResponse, &responseBuf, 0);
			}
		}
	}

	ESP_LOGE(TAG, "Shutting down socket");
	shutdown(sock, 0);
	close(sock);
	vTaskDelete(NULL);
}
