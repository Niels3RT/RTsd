#include "main.h"

static void httpn_server_task(void *pvParameters);

// ****** init HTTP
void oo_HTTP::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init HTTP server");
	
	// --- some vars
	tx_buf[0] = '\0';
	
	// --- Start the server for the first time
	xTaskCreate(httpn_server_task, "httpn_server", 4096, (void*)AF_INET, 5, NULL);
}

// ****** reply done
void oo_HTTP::reply_done(char *tbuf) {
	// --- reply done and new state
	sprintf(tbuf, "Done;%x\r\n", 0);
	
	//buf_reply[tx_len++] = 'D';
	//buf_reply[tx_len++] = 'o';
	//buf_reply[tx_len++] = 'n';
	//buf_reply[tx_len++] = 'e';
	//buf_reply[tx_len++] = ';';
	////buf_reply[tx_len++] = 0x30 + rt.state;
	//buf_reply[tx_len++] = '\r';
	//buf_reply[tx_len++] = '\n';
}

void oo_HTTP::do_retransmit(const int sock) {
    int len;
    char rx_buffer[128];
	char tx_buffer[128];
	char addr_str[128];
	
	struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
	
	// --- wait some for fpga to start up
	//vTaskDelay(3000 / portTICK_PERIOD_MS);

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
			
			struct sockaddr_in6 destAddr;
			unsigned socklen=sizeof(destAddr);
			getpeername(sock, (struct sockaddr *)&destAddr, &socklen);
			
			inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
			
			ESP_LOGI(TAG, "from peer: %d - %s", destAddr.sin6_addr.un.u32_addr[1], addr_str);
			//if(getpeername(sock, (struct sockaddr *)&destAddr, &socklen)<0) {
			//	ESP_LOGE(TAG, "getpeername failed, errno:%d",errno);
			//}
			
			tx_buffer[0] = 'N';
			tx_buffer[1] = 'i';
			tx_buffer[2] = 'e';
			tx_buffer[3] = 'l';
			tx_buffer[4] = 's';
			send(sock, tx_buffer, 5, 0);

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation. 
   //         int to_write = len;
   //         while (to_write > 0) {
   //             int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
   //             if (written < 0) {
   //                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
   //             }
   //             to_write -= written;
   //         }
        }
    } while (len > 0);
}

static void httpn_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

	while (1) {
		err = listen(listen_sock, 1);
		if (err != 0) {
			ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
			goto CLEAN_UP;
		}

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        http.do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}
