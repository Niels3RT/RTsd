#include "main.h"

//static const char *payload = "GET /results.csv:RTclient HTTP/1.1\r\nHost: 192.168.99.32\r\n\r\n";

// ****** init HTTP
void oo_HTTPC::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init HTTP client");
	
	// --- start httpc client thread
	start_client_thread();
}

// ****** find data
char * oo_HTTPC::find_data(char *tbuf) {
	// --- find double new line/cr, end of http header
	tbuf = strnstr(tbuf, "\r\n\r\n", sizeof(httpc.rx_buf));
	
	// --- retrun NULL if not found, otherwise add 4 to skip double crlf
	if (tbuf == NULL) {
		return(tbuf);
	} else {
		return(tbuf + 4);
	}
}

// ****** parse ascii hex in buffer to uint32_t
char * oo_HTTPC::parse_hex2uint32(char *ptmp) {
	uint32_t etmp = 0;
	bool gof = true;
	do {
		//printf("%c", *ptmp);
		if ((*ptmp >= 0x30) && (*ptmp < 0x3a)) {
			etmp <<= 4;
			etmp += *ptmp - 0x30;
			ptmp++;
		} else if((*ptmp > 0x40) && (*ptmp < 0x47)) {
			etmp <<= 4;
			etmp += *ptmp - 0x37;
			ptmp++;
		} else if((*ptmp > 0x60) && (*ptmp < 0x67)) {
			etmp <<= 4;
			etmp += *ptmp - 0x57;
			ptmp++;
		} else {
			gof = false;
		}
	} while(gof);
	t_etmp = etmp;
	return(ptmp);
}

// ****** parse csv line
uint8_t oo_HTTPC::parse_csv_line(char *ptmp) {
	uint8_t cnt = 0;
	uint32_t etmp = 0;
	// --- clear transfer array
	memset(csv_array, 0, sizeof(csv_array));
	// -- parse csv line
	bool go = true;
	bool gof = true;
	// -- walk through ';'
	do {
		// - parse and remember value
		etmp = 0;
		gof = true;
		do {
			//printf("%c", *ptmp);
			if ((*ptmp >= 0x30) && (*ptmp < 0x3a)) {
				etmp <<= 4;
				etmp += *ptmp - 0x30;
				ptmp++;
			} else if((*ptmp > 0x40) && (*ptmp < 0x47)) {
				etmp <<= 4;
				etmp += *ptmp - 0x37;
				ptmp++;
			} else if((*ptmp > 0x60) && (*ptmp < 0x67)) {
				etmp <<= 4;
				etmp += *ptmp - 0x57;
				ptmp++;
			} else {
				gof = false;
			}
		} while(gof);
		csv_array[cnt] = etmp;
		// - advance pointer?
		if ((*ptmp == 0x3b) && (cnt < 15)) {
			cnt++;
			ptmp++;
		} else {
			go = false;
		}
	} while(go);

	// --- return csv field counter
	return(cnt);
}

// ****** request pilotinfo
void oo_HTTPC::request_pilotinfo(void) {
	// --- increment ring buffer top counter
	tx_buf_top++;
	tx_buf_top &= 0x0f;
	
	// --- pointer
	char *tbuf_tx = &tx_buf[tx_buf_top][0];

	// --- request results
	strcpy(tbuf_tx, HTTP_REQ_PREFIX);		// prefix
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_GET_PILOTINFO);	// request
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_REQ_POST);			// post
	
	// --- set type in array
	msg_type[tx_buf_top] = TYPE_GET_PILOTINFO;
}

// ****** parse pilotinfo
bool oo_HTTPC::parse_pilotinfo(char *tbuf) {

	//printf("parse 'pilotinfo.csv'\r\n'%s'\r\n", tbuf);
	printf("parse 'pilotinfo.csv'\r\n");
	
	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- parse pilots
	if (tbuf != NULL) {
		uint8_t len = 0;
		info.pilot_count = 0;
		while(1) {
			// -- test if somewhat valid line
			if (strstr(tbuf, ";") == NULL) break;
			// -- parse field 0 for pilot nr
			tbuf = parse_hex2uint32(tbuf) + 1;
			info.pilots_data[info.pilot_count].nr = t_etmp;
			// -- copy pilot name
			len = strstr(tbuf, ";") - tbuf;
			strncpy(&info.pilots_data[info.pilot_count].name[0], tbuf, len);
			//printf("%d : '%s'\r\n", info.pilots_data[info.pilot_count].nr, info.pilots_data[info.pilot_count].name);
			info.pilot_count++;
			tbuf = strstr(tbuf, "\r\n");
			if (tbuf == NULL) break;
			tbuf += 2;
		}
	} else {
		// -- reply not good, retry
		return(false);
	}
	// --- retrun good
	return(true);
}

// ****** request raceinfo
void oo_HTTPC::request_raceinfo(void) {
	// --- increment ring buffer top counter
	tx_buf_top++;
	tx_buf_top &= 0x0f;
	
	// --- pointer
	char *tbuf_tx = &tx_buf[tx_buf_top][0];

	// --- request results
	strcpy(tbuf_tx, HTTP_REQ_PREFIX);		// prefix
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_GET_RACEINFO);		// request
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_REQ_POST);			// post
	
	// --- set type in array
	msg_type[tx_buf_top] = TYPE_GET_RACEINFO;
}

// ****** parse raceinfo
bool oo_HTTPC::parse_raceinfo(char *tbuf) {
	//printf("parse 'raceinfo.csv'\r\n'%s'\r\n", tbuf);
	printf("parse 'raceinfo.csv'\r\n");
	
	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- parse line as csv
	if (tbuf != NULL) {
		printf("%s", tbuf);
		// -- return good!
		return(true);
	} else {
		// -- return not good, retry
		return(false);
	}
}

// ****** request results
void oo_HTTPC::request_results(void) {
	// --- increment ring buffer top counter
	tx_buf_top++;
	tx_buf_top &= 0x0f;
	
	// --- pointer
	char *tbuf_tx = &tx_buf[tx_buf_top][0];

	// --- request results
	strcpy(tbuf_tx, HTTP_REQ_PREFIX);		// prefix
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_GET_RESULTS);		// request
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_REQ_POST);			// post
	
	// --- set type in array
	msg_type[tx_buf_top] = TYPE_GET_RESULTS;
}

// ****** parse results
bool oo_HTTPC::parse_results(char *tbuf) {
	char ctmp[200];

	//printf("parse 'results.csv'\r\n'%s'\r\n", tbuf);
	printf("parse 'results.csv'\r\n");

	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- parse line as csv
	if (tbuf != NULL) {
		int len = strstr(tbuf, "\r\n") - tbuf;
		uint8_t field_cnt = 0;
		while(len > 2) {
			strncpy(ctmp, tbuf, len);
			ctmp[len] = '\0';
			//printf("'%s'\r\n", ctmp);
			// -- parse line
			switch (*tbuf) {
				// is fastest lap line?
				case 'f':
					tbuf += 2;
					len -= 2;
					field_cnt = parse_csv_line(tbuf);
					break;
				// is position line?
				case 'p':
					tbuf += 2;
					len -= 2;
					field_cnt = parse_csv_line(tbuf);
					break;
				// default, parse line from byte 0
				default:
					field_cnt = parse_csv_line(tbuf);
					break;
			}
			// -- DBG: print csv fields
			//printf("csv: %d: ", field_cnt);
			//for (uint8_t i=0;i<field_cnt;i++) {
			//	printf("%x-", csv_array[i]);
			//}
			//printf("\r\n");
			// -- calc length of next line
			tbuf += len + 2;
			len = strstr(tbuf, "\r\n") - tbuf;
		}
		// -- return good!
		return(true);
	} else {
		// -- return not good, retry
		return(false);
	}
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
	//printf("send %x:\r\n'%s'\r\n", tx_buf_work, &tx_buf[tx_buf_work][0]);
	while (1) {
		err = send(sock, &tx_buf[tx_buf_work][0], strlen(&tx_buf[tx_buf_work][0]), 0);
		if (err < 0) {
			error_cnt++;
			if (error_cnt > 100) {
				printf("send failed: sock '%x'  err '%i' errno %d: '%s'\r\n", sock, err, errno, strerror(errno));
				return(-1);
				break;
			}
			vTaskDelay(10 / portTICK_PERIOD_MS);
		} else {
			//printf("send %d bytes ok!\r\n", err);
			return(err);
		}
	}
}

// ****** read reply
int oo_HTTPC::read_reply(int sock) {
	int len = 0;
	int error_cnt = 0;
	uint16_t rxpos = 0;
	memset(rx_buf, '\0', sizeof(rx_buf));
	while (1) {
		len = recv(sock, &rx_buf[rxpos], sizeof(rx_buf) - 1 - rxpos, 0);
		if (len >= 0) {
			rxpos += len;
			rx_buf[rxpos] = 0;
			if (len == 0) {
				return(rxpos);
			}
		} else {
			error_cnt++;
			if (error_cnt > 60) {
				printf("Read reply failed!\r\n");
				return(-1);
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
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
			httpc.tx_buf_work &= 0x0f;
		
			// --- create socket
			int sock = httpc.create_socket();
			if (sock > 0) {
				ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);
			}
			
			// -- connect socket
			int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
			//printf("connect: sock '%x'  err '%i' errno %d: '%s'\r\n", sock, err, errno, strerror(errno));
			
			// -- send request
			httpc.send_request(sock);
			
			// -- read reply
			if (httpc.read_reply(sock) == -1) {
				// - retry request
				httpc.tx_buf_work = tx_buf_use_retry;
			} else {
				// - parse reply
				bool it_worked = true;
				switch(httpc.msg_type[httpc.tx_buf_work]) {
					// pilotinfo
					case TYPE_GET_PILOTINFO:
						it_worked = httpc.parse_pilotinfo(&httpc.rx_buf[0]);
						break;
					// raceinfo
					case TYPE_GET_RACEINFO:
						it_worked = httpc.parse_raceinfo(&httpc.rx_buf[0]);
						break;
					// results
					case TYPE_GET_RESULTS:
						it_worked = httpc.parse_results(&httpc.rx_buf[0]);
						break;
				}
				// - something went wrong, retry
				if (!it_worked) httpc.tx_buf_work = tx_buf_use_retry;
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
	// --- start client task
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
