/*  BSD Socket TCP Client

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <netdb.h> //hostent

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "cmd.h"

#define PORT 6600 // MPD port

static const char *TAG = "TCP";

extern QueueHandle_t xQueueRequest;
extern QueueHandle_t xQueueResponse;

void tcp_client_task(void *pvParameters)
{
	char tx_buffer[128];
	char rx_buffer[128];
	char end_buffer[5];
	//end_buffer[0] = 0x0a; // LF
	end_buffer[0] = 0x4f; // O
	end_buffer[1] = 0x4b; // K
	end_buffer[2] = 0x0a; // LF
	end_buffer[3] = 0x00; 

	char host[] = CONFIG_ESP_MPD_SERVER;
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	dest_addr.sin_addr.s_addr = inet_addr(host);
	ESP_LOGI(TAG, "dest_addr.sin_addr.s_addr=0x%"PRIx32, dest_addr.sin_addr.s_addr);
	if (dest_addr.sin_addr.s_addr == 0xffffffff) {
		struct hostent *hp;
		hp = gethostbyname(host);
		if (hp == NULL) {
			ESP_LOGE(TAG, "gethostbyname fail");
			vTaskDelete(NULL);
		}
		struct ip4_addr *ip4_addr;
		ip4_addr = (struct ip4_addr *)hp->h_addr;
		dest_addr.sin_addr.s_addr = ip4_addr->addr;
	}
	ESP_LOGI(TAG, "dest_addr.sin_addr.s_addr=0x%"PRIx32, dest_addr.sin_addr.s_addr);
	
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
	}
	ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host, PORT);

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
			ESP_LOGI(TAG, "Received %d bytes from %s:", len, host);
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
				ESP_LOGD(TAG, "Received %d bytes from %s:", len, host);
				ESP_LOGD(TAG, "[%s]", rx_buffer);
				ESP_LOG_BUFFER_HEXDUMP(TAG, rx_buffer, len, ESP_LOG_DEBUG);
				payloadLength = payloadLength + len;
				if (response) {
					if (payloadLength < sizeof(responseBuf.payload)) {
						strcat(responseBuf.payload, rx_buffer);
					}
				}

				if (strstr(rx_buffer, end_buffer) != NULL) break;
				/* 
				This statement is incomplete.
				The letter O and the letter K will be received in separate packets.
				If the letter O and the letter K are split into separate packets, 
				the receive timeout will occur.
				*/
#if 0
receive this packet first:
The end is not "O" + "K" + LF, so the packet is not finish
I (126653) TCP: 0x3ffc8b00   76 6f 6c 75 6d 65 3a 20  34 31 0a 72 65 70 65 61  |volume: 41.repea|
I (126663) TCP: 0x3ffc8b10   74 3a 20 30 0a 72 61 6e  64 6f 6d 3a 20 30 0a 73  |t: 0.random: 0.s|
I (126663) TCP: 0x3ffc8b20   69 6e 67 6c 65 3a 20 30  0a 63 6f 6e 73 75 6d 65  |ingle: 0.consume|
I (126673) TCP: 0x3ffc8b30   3a 20 30 0a 70 6c 61 79  6c 69 73 74 3a 20 35 34  |: 0.playlist: 54|
I (126683) TCP: 0x3ffc8b40   0a 70 6c 61 79 6c 69 73  74 6c 65 6e 67 74 68 3a  |.playlistlength:|
I (126693) TCP: 0x3ffc8b50   20 31 30 0a 6d 69 78 72  61 6d 70 64 62 3a 20 30  | 10.mixrampdb: 0|
I (126703) TCP: 0x3ffc8b60   2e 30 30 30 30 30 30 0a  73 74 61 74 65 3a 20 70  |.000000.state: p|
I (126713) TCP: 0x3ffc8b70   6c 61 79 0a 73 6f 6e 67  3a 20 31 0a 73 6f 6e     |lay.song: 1.son|


receive this packet next:
The end is "O" + "K" + LF, so the packet is finished
I (126753) TCP: 0x3ffc8b00   67 69 64 3a 20 33 30 32  0a 74 69 6d 65 3a 20 31  |gid: 302.time: 1|
I (126763) TCP: 0x3ffc8b10   31 35 3a 32 31 31 0a 65  6c 61 70 73 65 64 3a 20  |15:211.elapsed: |
I (126763) TCP: 0x3ffc8b20   31 31 34 2e 35 39 34 0a  62 69 74 72 61 74 65 3a  |114.594.bitrate:|
I (126773) TCP: 0x3ffc8b30   20 33 32 30 0a 64 75 72  61 74 69 6f 6e 3a 20 32  | 320.duration: 2|
I (126783) TCP: 0x3ffc8b40   31 30 2e 37 32 39 0a 61  75 64 69 6f 3a 20 34 34  |10.729.audio: 44|
I (126793) TCP: 0x3ffc8b50   31 30 30 3a 32 34 3a 32  0a 6e 65 78 74 73 6f 6e  |100:24:2.nextson|
I (126803) TCP: 0x3ffc8b60   67 3a 20 32 0a 6e 65 78  74 73 6f 6e 67 69 64 3a  |g: 2.nextsongid:|
I (126813) TCP: 0x3ffc8b70   20 33 30 33 0a 4f 4b 0a                           | 303.OK.|
#endif
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
