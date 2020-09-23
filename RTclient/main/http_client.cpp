#include "main.h"

static const char *payload = "GET /results.csv:RTclient HTTP/1.1\r\nHost: 192.168.99.32\r\n\r\n";

// ****** init HTTP
void oo_HTTPC::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init HTTP client");
	
	// --- start httpc client thread
	start_client_thread();
}

// ****** reply done
void oo_HTTPC::reply_done(char *tbuf) {
	
}

// ****** create socket
int oo_HTTPC::create_socket(void) {
	// --- create socket
	int sock =  socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return(sock);
	}
	// --- set non-blocking
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	
	return(sock);
}

// ****** send request
int oo_HTTPC::send_request(int sock) {
	int error_cnt = 0;
	int err = 0;
	
	while (1) {
		//printf("send :\r\n%s", &tx_buf[tx_buf_work][0]);
		err = send(sock, &tx_buf[tx_buf_work][0], strlen(&tx_buf[tx_buf_work][0]), 0);
		//printf("send: sock '%x'  err '%i' errno %d: '%s'\r\n", sock, err, errno, strerror(errno));
		if (err < 0) {
			error_cnt++;
			if (error_cnt > 100) {
				printf("send failed: sock '%x'  err '%i' errno %d: '%s'\r\n", sock, err, errno, strerror(errno));
				return(-1);
				break;
			}
			vTaskDelay(10 / portTICK_PERIOD_MS);
		} else {
			printf("send %d bytes ok!\r\n", err);
			return(err);
		}
	}
}

// ****** read reply
int oo_HTTPC::read_reply(int sock) {
	int len = 0;
	int error_cnt = 0;
	uint16_t rxpos = 0;
	while (1) {
		len = recv(sock, &httpc.rx_buf[rx_buf_work][rxpos], sizeof(httpc.rx_buf[rx_buf_work]) - 1 - rxpos, 0);
		if (len >= 0) {
			rxpos += len;
			httpc.rx_buf[rx_buf_work][rxpos] = 0;
			if (len == 0) {
				printf("rxbuff:\r\n%s\r\n", httpc.rx_buf[rx_buf_work]);
				return(rxpos);
			}
		} else {
			error_cnt++;
			if (error_cnt > 10) {
				printf("Read reply failed!\r\n");
				return(-1);
			}
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

// ****** tcp client task
static void tcp_client_task(void *pvParameters) {
	struct sockaddr_in dest_addr;
	char host_ip[] = "192.168.99.32";
	uint8_t tx_buf_use_retry = 0;
	
	// --- fill sockaddr
	dest_addr.sin_addr.s_addr = inet_addr(host_ip);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	
	while(1) {
		if (httpc.tx_buf_work != httpc.tx_buf_top) {
			// --- increment buffer use signal, remember for retry
			tx_buf_use_retry = httpc.tx_buf_work;
			httpc.tx_buf_work++;
			if (httpc.tx_buf_work >= 16) httpc.tx_buf_work = 0;
			httpc.rx_buf_work = httpc.tx_buf_work;
		
			// --- create socket
			int sock = httpc.create_socket();
			if (sock > 0) {
				ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);
			}
			
			// -- connect socket
			int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
			printf("connect: sock '%x'  err '%i' errno %d: '%s'\r\n", sock, err, errno, strerror(errno));
			
			// -- send request
			httpc.send_request(sock);
			
			// -- read reply
			if (httpc.read_reply(sock) == -1) {
				httpc.tx_buf_work = tx_buf_use_retry;
			}
			
			// -- close socket
			shutdown(sock, 0);
			close(sock);
		} else {
			// -- sleep some
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
	}

    vTaskDelete(NULL);
}

// ****** start request
void oo_HTTPC::start_client_thread(void) {
	// --- copy request
	strcpy(&tx_buf[1][0], payload);
	httpc.tx_buf_work = 0;
	httpc.tx_buf_top = 1;

	// --- start client task
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
