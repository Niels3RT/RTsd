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
uint32_t oo_HTTPC::parse_hex2uint32(char *ptmp) {
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
	return(etmp);
}

// ****** split csv line
char * oo_HTTPC::split_csv_line(char *ptmp) {
	csv_split_cnt = 0;
	char * ptest = 0;
	char * pend = 0;
	uint8_t len = 0;
	// --- clear transfer array
	memset(csv_array_str, 0, sizeof(csv_array_str));
	// --- find end of line
	ptest = strstr(ptmp, "\r\n");
	if (ptest == NULL) {
		return(ptest);
	} else {
		pend = ptest;
	}
	// -- parse csv line, walk through ';'
	while(1) {
		// - find next ';', copy field
		ptest = strstr(ptmp, ";");
		if (ptest == NULL) {
			break;
		} else {
			if (ptest >= pend) {
				break;
			}
			len = ptest - ptmp;
			strncpy(&csv_array_str[csv_split_cnt][0], ptmp, len);
			ptmp = ptest + 1;
			csv_split_cnt++;
		}
	};

	// --- return pointer to next line
	return(ptmp+2);
}

// ****** request config
void oo_HTTPC::request_config(void) {
	// --- increment ring buffer top counter
	tx_buf_top++;
	tx_buf_top &= 0x0f;
	
	// --- pointer
	char *tbuf_tx = &tx_buf[tx_buf_top][0];

	// --- request results
	strcpy(tbuf_tx, HTTP_REQ_PREFIX);		// prefix
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_GET_CFG);			// request
	tbuf_tx += strlen(tbuf_tx);
	strcpy(tbuf_tx, HTTP_REQ_POST);			// post
	
	// --- set type in array
	msg_type[tx_buf_top] = TYPE_GET_CFG;
}

// ****** parse config
bool oo_HTTPC::parse_config(char *tbuf) {
	printf("parse 'config'\r\n'%s'\r\n", tbuf);
	//printf("parse 'config'\r\n");
	
	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- parse pilots
	uint8_t cnt_line = 0;
	if (tbuf != NULL) {
		// -- walk through lines
		tbuf = split_csv_line(tbuf);
		while(tbuf != NULL) {
			//printf("%d: '%s;%s;%s;'\r\n", cnt_line, csv_array_str[0], csv_array_str[1], csv_array_str[2]);
			switch(cnt_line) {
				// - line 1, max chn
				case 1:
					cfg.max_chn = parse_hex2uint32(&csv_array_str[0][0]);
					break;
			}
			cnt_line++;
			tbuf = split_csv_line(tbuf);
		}
		// -- return good!
		return(true);
	} else {
		// -- return not good, retry
		return(false);
	}
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
	info.pilot_count = 0;
	if (tbuf != NULL) {
		//printf("%s", tbuf);
		// -- walk through lines
		tbuf = split_csv_line(tbuf);
		while(tbuf != NULL) {
			// - pilot nr
			info.pilots_data[info.pilot_count].nr = parse_hex2uint32(&csv_array_str[0][0]);
			// - pilot name
			memset(info.pilots_data[info.pilot_count].name, '\0', sizeof(info.pilots_data[info.pilot_count].name));
			strncpy(info.pilots_data[info.pilot_count].name, csv_array_str[1], sizeof(info.pilots_data[info.pilot_count].name)-1);
			//printf("%d : '%s'\r\n", info.pilots_data[info.pilot_count].nr, info.pilots_data[info.pilot_count].name);
			info.pilot_count++;
			tbuf = split_csv_line(tbuf);
		}
		// -- return good!
		return(true);
	} else {
		// -- return not good, retry
		return(false);
	}
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
	uint8_t cnt_line = 0;
	//printf("parse 'raceinfo.csv'\r\n'%s'\r\n", tbuf);
	printf("parse 'raceinfo.csv'\r\n");
	
	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- parse line as csv
	if (tbuf != NULL) {
		//printf("%s", tbuf);
		// -- walk through lines
		tbuf = split_csv_line(tbuf);
		while(tbuf != NULL) {
			switch(cnt_line) {
				// - line 0, state, heat mod_cnt
				case 0:
					info.state = parse_hex2uint32(&csv_array_str[0][0]);
					info.heat_mod_cnt = parse_hex2uint32(&csv_array_str[1][0]);
					printf("state: '%d'\r\nheat_mod_cnt: '%d'\r\n", info.state, info.heat_mod_cnt);
					break;
				// - line 1, event
				case 1:
					// event is open
					if (parse_hex2uint32(&csv_array_str[0][0]) == 1) {
						info.event_is_open = true;
					} else {
						info.event_is_open = false;
					}
					// event name
					memset(info.event.name, '\0', sizeof(info.event.name));
					strncpy(info.event.name, csv_array_str[3], sizeof(info.event.name)-1);
					//sprintf(info.event.name, "%s", csv_array_str[3]);
					break;
				// - line 2, session
				case 2:
					// session is open
					if (parse_hex2uint32(&csv_array_str[0][0]) == 1) {
						info.session_is_open = true;
					} else {
						info.session_is_open = false;
					}
					// session name
					memset(info.session.name, '\0', sizeof(info.session.name));
					strncpy(info.session.name, csv_array_str[3], sizeof(info.session.name)-1);
					break;
				// - line 3, heat
				case 3:
					// heat is open
					if (parse_hex2uint32(&csv_array_str[0][0]) == 1) {
						info.heat_is_open = true;
					} else {
						info.heat_is_open = false;
					}
					// heat name
					memset(info.heat.name, '\0', sizeof(info.heat.name));
					strncpy(info.heat.name, csv_array_str[3], sizeof(info.heat.name)-1);
					// pilots nr
					info.heat.pilots_nr[0] = parse_hex2uint32(&csv_array_str[4][0]);
					info.heat.pilots_nr[1] = parse_hex2uint32(&csv_array_str[5][0]);
					info.heat.pilots_nr[2] = parse_hex2uint32(&csv_array_str[6][0]);
					info.heat.pilots_nr[3] = parse_hex2uint32(&csv_array_str[7][0]);
					info.heat.pilots_nr[4] = parse_hex2uint32(&csv_array_str[8][0]);
					info.heat.pilots_nr[5] = parse_hex2uint32(&csv_array_str[9][0]);
					info.heat.pilots_nr[6] = parse_hex2uint32(&csv_array_str[10][0]);
					info.heat.pilots_nr[7] = parse_hex2uint32(&csv_array_str[11][0]);
					break;
			}
			cnt_line++;
			tbuf = split_csv_line(tbuf);
		}
		// -- pilots ok?
		for (uint8_t i=0;i<cfg.max_chn;i++) {
			if (info.heat.pilots_nr[i] == 0xff) info.heat.pilots_nr[i] = i;
		}
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
	uint8_t cnt_line = 0;

	//printf("parse 'results.csv'\r\n'%s'\r\n", tbuf);
	printf("parse 'results.csv'\r\n");

	// --- jump http header to data
	tbuf = find_data(tbuf);
	
	// --- clear some stuff
	memset(info.hitcount, 0, sizeof(info.hitcount));
	memset(info.hits, 0, sizeof(info.hits));
	
	// --- parse line as csv
	if (tbuf != NULL) {
		// -- walk through lines
		tbuf = split_csv_line(tbuf);
		while(tbuf != NULL) {
			if (cnt_line == 0) {
				// - line 0
				info.timestamp = parse_hex2uint32(&csv_array_str[0][0]);
				info.state = parse_hex2uint32(&csv_array_str[1][0]);
				if (cfg.max_chn == 4) {
					info.use_event_mode = parse_hex2uint32(&csv_array_str[7][0]);
					info.event_mod_cnt_new = parse_hex2uint32(&csv_array_str[8][0]);
					info.session_mod_cnt_new = parse_hex2uint32(&csv_array_str[9][0]);
					info.heat_mod_cnt_new = parse_hex2uint32(&csv_array_str[10][0]);
				} else {
					info.use_event_mode = parse_hex2uint32(&csv_array_str[11][0]);
					info.event_mod_cnt_new = parse_hex2uint32(&csv_array_str[12][0]);
					info.session_mod_cnt_new = parse_hex2uint32(&csv_array_str[13][0]);
					info.heat_mod_cnt_new = parse_hex2uint32(&csv_array_str[14][0]);
				}
			} else {
				switch(csv_array_str[0][0]) {
					// - hit
					case 'h':
						info.hits[parse_hex2uint32(&csv_array_str[1][0])][parse_hex2uint32(&csv_array_str[2][0])] = parse_hex2uint32(&csv_array_str[3][0]);
						info.hitcount[parse_hex2uint32(&csv_array_str[1][0])] = parse_hex2uint32(&csv_array_str[2][0]) + 1;
						break;
					// - fastest laps
					case 'f':
						break;
					// - position
					case 'p':
						break;
				}
			}
			cnt_line++;
			tbuf = split_csv_line(tbuf);
		}
		// --- send event to main loop
		ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_USE_RESULT, NULL, 0, portMAX_DELAY));
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
			connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
			//int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
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
					// get config
					case TYPE_GET_CFG:
						it_worked = httpc.parse_config(&httpc.rx_buf[0]);
						break;
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
