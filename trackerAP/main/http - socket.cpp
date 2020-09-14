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
	//uint addr_len = sizeof(source_addr);
	
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
    ESP_LOGI(TAG,"tcp_server task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(80);
    int s, r;
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);
    int cs;//client socket
    //xEventGroupWaitBits(wifi_event_group,AP_STARTED_BIT,false,true,portMAX_DELAY);
    while(1){
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
         if(bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket bind failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket bind done \n");
        if(listen (s, LISTENQ) != 0) {
            ESP_LOGE(TAG, "... socket listen failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        while(1){
            cs=accept(s,(struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG,"New connection request,Request data:");
            //set O_NONBLOCK so that recv will return, otherwise we need to impliment message end 
            //detection logic. If know the client message format you should instead impliment logic
            //detect the end of message
            fcntl(cs,F_SETFL,O_NONBLOCK);
            do {
                bzero(recv_buf, sizeof(recv_buf));
                r = recv(cs, recv_buf, sizeof(recv_buf)-1,0);
                for(int i = 0; i < r; i++) {
                    putchar(recv_buf[i]);
                }
            } while(r > 0);
            
            ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
			
            
            if( write(cs , HEADER , strlen(HEADER)) < 0)
            {
                ESP_LOGE(TAG, "... Send failed \n");
                close(s);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }
            ESP_LOGI(TAG, "... socket send success");
            close(cs);
        }
        ESP_LOGI(TAG, "... server will be opened in 5 seconds");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "...tcp_client task closed\n");
}
