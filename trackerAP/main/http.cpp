#include "main.h"

static void httpn_server_task(void *pvParameters);

// ****** init HTTP
void oo_HTTP::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init HTTP server");
	
	// --- some vars
	tx_buf[0][0] = '\0';
	tx_buf[1][0] = '\0';
	tx_buf[2][0] = '\0';
	tx_buf[3][0] = '\0';
	rxpos[0] = 0;
	rxpos[1] = 0;
	rxpos[2] = 0;
	rxpos[3] = 0;
	
	// --- Start the server for the first time
	xTaskCreate(httpn_server_task, "httpn_server", 4096, (void*)AF_INET, 5, NULL);
}

// ****** reply done
void oo_HTTP::reply_done(char *tbuf) {
	// --- reply done and new state
	sprintf(tbuf, "Done;%x\r\n", rt.state);
}

// ****** get config
void oo_HTTP::reply_get_cfg(char *tbuf) {
	// --- max channels
	sprintf(tbuf, "%04x;\r\n", rt.max_chn);
	tbuf += strlen(tbuf);
	// --- rx frequencies
	if (rt.max_chn == 4) {
		sprintf(tbuf, "%04x;%04x;%04x;%04x;\r\n", rt.chn_freq[0], rt.chn_freq[1], rt.chn_freq[2], rt.chn_freq[3]);
	}
	if (rt.max_chn == 8) {
		sprintf(tbuf, "%04x;%04x;%04x;%04x;%04x;%04x;%04x;%04x;\r\n", rt.chn_freq[0], rt.chn_freq[1], rt.chn_freq[2], rt.chn_freq[3], rt.chn_freq[4], rt.chn_freq[5], rt.chn_freq[6], rt.chn_freq[7]);
	}
	tbuf += strlen(tbuf);
	// --- deadtime
	sprintf(tbuf, "%04x;", rt.deadtime);
	tbuf += strlen(tbuf);
	// --- detection auto percentage
	sprintf(tbuf, "%04x;", rt.det_quot_perc);
	tbuf += strlen(tbuf);
	// --- detection auto minimum level
	sprintf(tbuf, "%04x;", rt.det_auto_min);
	tbuf += strlen(tbuf);
	// --- rssi moving window filter
	sprintf(tbuf, "%04x;", 5);
}

// ****** set config
void oo_HTTP::reply_set_cfg(char *tbuf_header, char *tbuf_tx) {
	// --- fetch rx frequencies
	uint8_t pos = 13;
	for (uint8_t i=0;i<rt.max_chn;i++) {
		rt.chn_freq[i] = buf.buf2uint16_t((uint8_t*)tbuf_header + pos);
		printf("freq %d: %d\r\n", i, rt.chn_freq[i]);
		pos += 5;
	}
	// --- fetch deadtime
	rt.deadtime = buf.buf2uint16_t((uint8_t*)tbuf_header + pos);
	printf("dead: %d\r\n", rt.deadtime);
	pos += 5;
	// --- fetch detect level quotient
	rt.det_quot_perc = buf.buf2uint16_t((uint8_t*)tbuf_header + pos);
	printf("quot: %d\r\n", rt.det_quot_perc);
	
	// --- tune rxes
	rt.tune_rx_all();
	// --- set deadtime
	rt.pd_set_deadtime();
	// --- set detect level quotient
	rt.pd_set_auto_quotient();

	// --- write response
	strcat(tbuf_tx, "set_cfg ");
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** reply results
void oo_HTTP::reply_results(char *tbuf) {
	// --- peak detect
	if (rt.pd_isready()) {
		// -- fetch results from pd memory
		rt.pd_fetch();
		// -- start peak detect run
		rt.pd_start();
	}
	
	// --- calc ranking
	heat.calc_laps();
	heat.calc_fastest_laps();
	heat.calc_position();

	// --- print timestamp
	sprintf(tbuf, "%08x;", rt.count);
	oled.print_hex_32u(8, 0, rt.count);
	oled.writefb();
	
	// --- print state
	rt.get_state();
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%x;", rt.state);
	
	// --- detection levels
	rt.pd_get_tlevels();
	for (uint8_t i=0;i<rt.max_chn;i++) {
		tbuf += strlen(tbuf);
		sprintf(tbuf, "%03x;", rt.trg_level[i]);
	}
	
	// --- print exception mod counter
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%02x;", rt.ex_mod_cnt);
	
	// --- print use event mode
	tbuf += strlen(tbuf);
	uint8_t otmp = 0;
	if (heat.is_open) otmp += 4;
	if (session.is_open) otmp += 2;
	if (event.is_open) otmp += 1;
	sprintf(tbuf, "%x;", otmp);
	
	// --- print event mod counter
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%02x;", event.mod_cnt);
	
	// --- print session mod counter
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%02x;", session.mod_cnt);
	
	// --- print heat mod counter
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%02x;", heat.mod_cnt);
	
	// --- finish line
	tbuf += strlen(tbuf);
	sprintf(tbuf, "\r\n");
	
	// --- print hits
	for (uint8_t k=0;k<rt.max_chn;k++) {
		for (uint8_t i=0;i<rt.hitcount[k];i++) {
			tbuf += strlen(tbuf);
			sprintf(tbuf, "h;%d;%02x;%08x;\r\n", k, i, rt.hits[k][i]);
		}
	}
	
	// --- print fastest laps in heat, order by fastest
	for (uint8_t i=0;i<rt.max_chn;i++) {
		tbuf += strlen(tbuf);
		sprintf(tbuf, "f;%d;%02x;%02x;\r\n", i, heat.current.pos_fastest_lap[i], heat.current.fastest_laps_lapnr[heat.current.pos_fastest_lap[i]]);
	}
	
	// --- print position in heat
	for (uint8_t i=0;i<rt.max_chn;i++) {
		tbuf += strlen(tbuf);
		sprintf(tbuf, "p;%d;%02x;%02x;%08x;\r\n", i, heat.current.pos_nr[i], heat.current.lapcount[heat.current.pos_nr[i]], heat.current.heat_time[heat.current.pos_nr[i]]);
	}
}

// ****** reply race info
void oo_HTTP::reply_raceinfo(char *tbuf) {
	// --- line 0
	// -- print state, mod counter
	rt.get_state();
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%x;%x;\r\n", rt.state, heat.mod_cnt);
	
	// --- line 1, event
	tbuf += strlen(tbuf);
	if (event.is_open) {
		sprintf(tbuf, "%x;%x;%x;%s;\r\n", event.is_open, event.nr, event.events_cnt, event.name);
	} else {
		sprintf(tbuf, "%x;0;0;;\r\n", event.is_open);
	}
	
	// --- line 2, session
	tbuf += strlen(tbuf);
	if (session.is_open) {
		sprintf(tbuf, "%x;%x;%x;%s;\r\n", session.is_open, session.nr, event.sessions_cnt, session.name);
	} else {
		if (event.is_open) {
			sprintf(tbuf, "%x;0;%x;;\r\n", session.is_open, event.sessions_cnt);
		} else {
			sprintf(tbuf, "%x;0;0;;\r\n", session.is_open);
		}
	}
	
	// --- line 3, heat
	tbuf += strlen(tbuf);
	sprintf(tbuf, "%x;%x;%x;%s;%02x;%02x;%02x;%02x;\r\n", heat.is_open, heat.current.nr, session.heat_cnt, heat.current.name, heat.current.pilots_nr[0], heat.current.pilots_nr[1], heat.current.pilots_nr[2], heat.current.pilots_nr[3]);
}

// ****** reply session info
void oo_HTTP::reply_sessioninfo(char *tbuf) {
	// --- walk through sessions
	for (uint8_t h=0;h<session.heat_cnt;h++) {
		for (uint8_t c=0;c<rt.max_chn;c++) {
			tbuf += strlen(tbuf);
			sprintf(tbuf, "%x;%x;%x;%08x;%08x;\r\n", session.heats[h].nr,
										c,
										session.heats[h].lapcount[c],
										session.heats[h].heat_time[c],
										session.heats[h].fastest_laps_time[c]);
		}
	}
}

// ****** reply pilot info
void oo_HTTP::reply_pilotinfo(char *tbuf) {
	// --- walk through pilots
	for (uint8_t i=0;i<event.pilots_all_cnt;i++) {
		// -- print pilot info
		tbuf += strlen(tbuf);
		sprintf(tbuf, "%02x;%s;\r\n", event.pilots_data[i].nr, event.pilots_data[i].name);
	}
}

// ****** reply rssi
void oo_HTTP::reply_rssi(char *tbuf_rx, char *tbuf_tx) {
	// --- fetch start timestamp and count
	uint32_t tmp_start = buf.buf2uint32_t((uint8_t*)(tbuf_rx+14));
	uint16_t tmp_count = buf.buf2uint16_t((uint8_t*)(tbuf_rx+23));
	
	// --- write response
	char trssi[16];
	uint8_t pos = 0;
	uint16_t tmp = 0;
	for (uint16_t i=0;i<tmp_count;i++) {
		// -- print timestamp
		tbuf_tx += strlen(tbuf_tx);
		sprintf(tbuf_tx, "%06x;", tmp_start+i);
		rtspi.transmit24(RT_SDRAM2REG, tmp_start+i, 0);		// set sdram address (rssi block nr)
		switch(rt.max_chn) {
			case 4:
				rtspi.read64(&trssi[0]);							// fetch 64bit word from transfer register
				break;
			case 8:
				rtspi.read128(&trssi[0]);							// fetch 128bit word from transfer register
				break;
		}
		// -- print line to output buffer
		pos = 0;
		tmp = 0;
		for (uint8_t k=0;k<rt.max_chn;k++) {
			tbuf_tx += strlen(tbuf_tx);
			tmp = ((trssi[pos]<<8) + trssi[pos+1]) & 0xfff;
			sprintf(tbuf_tx, "%03x;", tmp);
			pos += 2;
		}
		strcat(tbuf_tx, "\r\n");
	}
}

// ****** set trigger level
void oo_HTTP::reply_tlevel(char *tbuf_rx, char *tbuf_tx) {
	// --- fetch and set trigger levels
	uint8_t pos = 15;
	for (uint8_t i=0;i<rt.max_chn;i++) {
		rt.pd_set_tlevel(i, buf.buf2uint16_t((uint8_t*)(tbuf_rx+pos)));
		pos += 5;
	}

	// --- set fixed trigger mode
	rt.pd_set_fixed_mode(0x0f);
	
	// -- start peak detect run
	rt.pd_start();
	
	// --- write response
	strcat(tbuf_tx, "level_set ");
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
	
	// --- handle heat state
	if (heat.is_open) {
		printf("Set trigger levels, Heat is open!\r\n");
		if (rt.state == 3) {
			printf("Set trigger levels, rt.state is 3\r\n");
			rt.state = 2;
			rt.set_state();
		}
	}
}

// ****** get min max for calibration
void oo_HTTP::reply_get_minmax(char *tbuf_tx) {
	// --- get min max from fpga
	rt.fetch_minmax();
	// --- fetch and send data, renew if in calibration mode
	if (rt.do_calib) {
		// -- recalc calibration data
		for (uint8_t i=0;i<rt.max_chn;i++) {
			rt.rssi_quot[i] = ((float)(rt.rssi_max[i] - rt.rssi_min[i])) / 4096;
			if (rt.rssi_quot[i] < 0.5f) rt.rssi_quot[i] = 0.5f;
			rt.rssi_base[i] = rt.rssi_min[i];
			
			printf("min %04x  max %04x  delta %05d  quot %2f\r\n", rt.rssi_min[i], rt.rssi_max[i], rt.rssi_max[i] - rt.rssi_min[i], rt.rssi_quot[i]);
		}
		// -- update fpga if calibration is running
		rt.write_cal_data();
	}
	// --- print min max to tx buffer
	for (uint8_t i=0;i<rt.max_chn;i++) {
		sprintf(tbuf_tx, "%x;%05x;%05x\r\n", i, rt.rssi_min[i], rt.rssi_max[i]);
		tbuf_tx += strlen(tbuf_tx);
	}
	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** get exceptions
void oo_HTTP::reply_get_ex(char *tbuf_tx) {
	// --- send data
	for (uint8_t i=0;i<rt.max_chn;i++) {
		for (uint8_t k=0;k<rt.excount[i];k++) {
			sprintf(tbuf_tx, "%x;%08x;\r\n", i, rt.exceptions[i][k]);
			tbuf_tx += strlen(tbuf_tx);
			
			printf("chn %d  pos %08x\r\n", i, rt.exceptions[i][k]);
		}
	}
	
	// --- write response
	sprintf(tbuf_tx, "%x;%x\r\n", rt.ex_mod_cnt, rt.state);
}

// ****** get available events
void oo_HTTP::reply_get_events(char *tbuf_tx) {
	// --- open event directory
	if (sd.dir_open(HTTP_SD_PATH_EVENTS)) {
		// -- send data
		event.events_cnt = 0;
		sd.dir_get_entry();
		while(sd.entry) {
			if (sd.entry->d_type == DT_DIR) {
				sprintf(tbuf_tx, "%s;\r\n", sd.entry->d_name);
				tbuf_tx += strlen(tbuf_tx);
				event.events_cnt++;
			}
			sd.dir_get_entry();
		}
		// -- close directory
		sd.dir_close();
	}
}

// ****** open event
void oo_HTTP::reply_open_event(char *tbuf_rx, char *tbuf_tx) {
	// --- fetch event nr to open
	char * ptmp = strchr(tbuf_rx, ';') + 1;
	uint8_t etmp = 0;
	while((*ptmp >= 0x30) && (*ptmp < 0x3a)) {
		etmp *= 10;
		etmp += *ptmp - 0x30;
		ptmp++;
	}
	
	// --- open event
	event.open(etmp);

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** get event pilots
void oo_HTTP::reply_get_event_pilots(char *tbuf_tx) {
	// --- walk through pilots
	for (uint8_t i=0;i<event.pilots_cnt;i++) {
		// -- print pilot
		sprintf(tbuf_tx, "%x;%x;\r\n", i, event.pilots[i]);
		tbuf_tx += strlen(tbuf_tx);
	}
}

// ****** set event pilots
void oo_HTTP::reply_set_event_pilots(char *tbuf_rx, char *tbuf_tx) {
	// --- walk through pilots
	event.pilots_cnt = 0;
	char * ptmp = tbuf_rx;
	while((*ptmp >= 0x30) && (*ptmp <= 0x39)) {
		event.pilots[event.pilots_cnt] = buf.buf2uintX_t_dec(ptmp);
		event.pilots_cnt++;
		ptmp = strchr(ptmp, ';') + 1;
	}
	// --- write event pilots to data file
	event.write_event_pilots();
}

// ****** close event
void oo_HTTP::reply_close_event(char *tbuf_tx) {
	// --- close event
	event.close();

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** event results
void oo_HTTP::reply_event_results(char *tbuf_tx) {
	// --- write event results
	printf("Send Event Results!\r\n");
	
	// --- fastest laps
	for(uint8_t i=0;i<CFG_MAX_PILOTS;i++) {
		if (event.fastest_laps[i].pilot_nr != 0xffff) {
			tbuf_tx += strlen(tbuf_tx);
			sprintf(tbuf_tx, "0;%x;%08x;\r\n", event.fastest_laps[i].pilot_nr, event.fastest_laps[i].time);
		}
	}
	// --- send quali results
	for(uint8_t i=0;i<CFG_MAX_PILOTS;i++) {
		if (event.quali_time[i].pilot_nr != 0xffff) {
			tbuf_tx += strlen(tbuf_tx);
			sprintf(tbuf_tx, "1;%x;%x;%08x;\r\n", event.quali_time[i].pilot_nr, event.quali_time[i].laps, event.quali_time[i].time);
		}
	}

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** create new event
void oo_HTTP::reply_new_event(char *tbuf_rx, char *tbuf_tx) {
	//st_event evtmp = event.empty;
	printf("new event:\r\n%s", tbuf_rx);
	st_event evtmp;
	// --- parse new name
	char * ptmp = tbuf_rx;
	uint8_t len = strchr(ptmp, ';') - ptmp;
	if (len > 39) len = 39;
	strncpy(&evtmp.name[0], ptmp, len);
	evtmp.name[len] = '\0';
	// --- quali mode
	ptmp = strchr(ptmp, ';') + 1;
	evtmp.quali_mode = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	// --- quali laps
	ptmp = strchr(ptmp, ';') + 1;
	evtmp.quali_laps = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	// --- quali otime
	ptmp = strchr(ptmp, ';') + 1;
	evtmp.quali_otime = (uint16_t)buf.buf2uintX_t_dec(ptmp);
	// --- race mode
	ptmp = strchr(ptmp, ';') + 1;
	evtmp.race_mode = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	// --- race laps
	ptmp = strchr(ptmp, ';') + 1;
	evtmp.race_laps = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	
	printf("Create new Event: '%s'\r\n%d:%d:%d\r\n", evtmp.name, evtmp.quali_mode, evtmp.quali_laps, evtmp.quali_otime);
	printf("%d:%d\r\n", evtmp.race_mode, evtmp.race_laps);
	
	// --- create new event
	event.create_new(&evtmp);

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** get available sessions in current event
void oo_HTTP::reply_get_sessions(char *tbuf_tx) {
	// -- send data
	for (uint8_t i=0;i<event.sessions_cnt;i++) {
		sprintf(tbuf_tx, "%d;%s;\r\n", event.sessions[i].nr, event.sessions[i].name);
		tbuf_tx += strlen(tbuf_tx);
	}
}

// ****** open session
void oo_HTTP::reply_open_session(char *tbuf_rx, char *tbuf_tx) {
	// --- fetch session nr to open
	char * ptmp = strchr(tbuf_rx, ';') + 1;
	uint8_t etmp = 0;
	while((*ptmp >= 0x30) && (*ptmp < 0x3a)) {
		etmp *= 10;
		etmp += *ptmp - 0x30;
		ptmp++;
	}
	printf("SessionNr '%d'\r\n", etmp);
	
	// --- open event
	session.open(etmp);

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** get session pilots
void oo_HTTP::reply_get_session_pilots(char *tbuf_tx) {
	// --- walk through heats
	for (uint8_t i=0;i<session.heat_cnt;i++) {
		for (uint8_t k=0;k<rt.max_chn;k++) {
			// -- print pilot, if is real
			if (session.heats[i].pilots_nr[k] != 0xffff) {
				sprintf(tbuf_tx, "%x;%x;%x;\r\n", i, k, session.heats[i].pilots_nr[k]);
				tbuf_tx += strlen(tbuf_tx);
			}
		}
	}
}

// ****** close session
void oo_HTTP::reply_close_session(char *tbuf_tx) {
	// --- close event
	session.close();

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** create new session
void oo_HTTP::reply_new_session(char *tbuf_rx, char *tbuf_tx) {
	printf("new session:\r\n%s", tbuf_rx);
	// --- session mode
	char * ptmp = tbuf_rx;
	uint8_t sess_mode = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	// --- quali laps
	ptmp = strchr(ptmp, ';') + 1;
	uint8_t gen_mode = (uint8_t)buf.buf2uintX_t_dec(ptmp);
	
	// --- create new session
	session.create_new(sess_mode, gen_mode);

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** open heat
void oo_HTTP::reply_open_heat(char *tbuf_rx, char *tbuf_tx) {
	// --- fetch heat nr to open
	char * ptmp = strchr(tbuf_rx, ';') + 1;
	uint8_t etmp = 0;
	while((*ptmp >= 0x30) && (*ptmp < 0x3a)) {
		etmp *= 10;
		etmp += *ptmp - 0x30;
		ptmp++;
	}
	
	// --- open heat
	heat.current.nr = etmp;
	ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_OPEN, NULL, 0, portMAX_DELAY));

	// --- write response
	http.reply_done(tbuf_tx + strlen(tbuf_tx));
}

// ****** send file, returns true if ok
bool oo_HTTP::reply_file(char *tbuf_rx, int tcs, int buf_nr) {
	char filename[256];
	int len = 0;
	bool is_good = true;
	FILE* tmpfile;
	
	// --- try to fetch/build filename
	tbuf_rx = strstr(tbuf_rx, " ") + 1;
	len = strstr(tbuf_rx, " ") - tbuf_rx;
	bzero(filename, sizeof(filename));
	sprintf(&filename[0], "%s", HTTP_SD_PATH);
	strncpy(&filename[strlen(&filename[0])], tbuf_rx, len);
	
	// --- decide on content type
	if (strstr(tbuf_rx, ".html"))
		strcpy(&http.tx_buf[buf_nr][strlen(http.tx_buf[buf_nr])], HTTP_CONTENT_HTML);
	if (strstr(tbuf_rx, ".js"))
		strcpy(&http.tx_buf[buf_nr][strlen(http.tx_buf[buf_nr])], HTTP_CONTENT_JS);
	if (strstr(tbuf_rx, ".css"))
		strcpy(&http.tx_buf[buf_nr][strlen(http.tx_buf[buf_nr])], HTTP_CONTENT_CSS);
	if (strstr(tbuf_rx, ".png"))
		strcpy(&http.tx_buf[buf_nr][strlen(http.tx_buf[buf_nr])], HTTP_CONTENT_PNG);
	if (strstr(tbuf_rx, ".jpg"))
		strcpy(&http.tx_buf[buf_nr][strlen(http.tx_buf[buf_nr])], HTTP_CONTENT_JPG);
	
	// --- open file
	tmpfile = fopen(filename, "r");
	if (tmpfile != NULL) {
		// -- file opened, send
		ESP_LOGI(TAG, "file '%s' found, start sending", filename);
		int rsize = 0;
		int sent_size = 0;
		int send_size = strlen(http.tx_buf[buf_nr]);
		char * sbuf = tbuf_rx;
		int cnt_err = 0;
		do {
			rsize = (int)fread(&tx_buf[buf_nr][0]+send_size, 1, sizeof(tx_buf[buf_nr])-send_size, tmpfile);
			send_size += rsize;
			rsize = send_size;
			sbuf = &tx_buf[buf_nr][0];
			cnt_err = 0;
			do {
				sent_size = send(tcs, sbuf, send_size, 0);
				if (sent_size > 0) {
					send_size -= sent_size;
					sbuf += sent_size;
					cnt_err = 0;
				} else {
					vTaskDelay(10 / portTICK_PERIOD_MS);
					if (cnt_err < 80) {
						cnt_err++;
					} else {
						// - too many fails, cut it off
						send_size = 0;
						rsize = 0;
					}
				}
			} while(send_size > 0);
			printf("send_size %d rsize %d sizeof %d\r\n", send_size, rsize, sizeof(tx_buf[buf_nr]));
		} while(rsize == sizeof(tx_buf[buf_nr]));
		
		fclose(tmpfile);
	} else {
		// -- failed to open file, 404
		ESP_LOGI(TAG, "file '%s' not found, reply 404", filename);
		is_good = false;
	}
	
	return(is_good);
}

// ****** parse exceptions
void oo_HTTP::parse_ex(char *tbuf) {
	// --- clear exceptions array
	for (uint8_t i=0;i<rt.max_chn;i++) rt.excount[i] = 0;
	// --- walk through lines and read into array
	uint8_t utmp = *tbuf - 0x30;
	uint32_t ptmp = 0;
	while((utmp < rt.max_chn) && (strlen(tbuf) > 10)) {
		utmp = *tbuf - 0x30;
		ptmp = buf.buf2uint32_t((uint8_t*)(tbuf+2));
		printf("%x - %08x\r\n", utmp, ptmp);
		rt.exceptions[utmp][rt.excount[utmp]] = ptmp;
		rt.excount[utmp]++;
		tbuf = strstr(tbuf, "\r\n") + 2;
	}
	// --- send exceptions array to fpga
	rt.pd_set_exceptions();
	// --- increment exception mod counter
	rt.ex_mod_cnt++;
	// --- handle heat state
	if (heat.is_open) {
		printf("Exception, Heat is open!\r\n");
		if (rt.state == 3) {
			printf("Exception, rt.state is 3\r\n");
			rt.state = 2;
			rt.set_state();
		}
	}
}

// ****** http server task
static void httpn_server_task(void *pvParameters)
{
    ESP_LOGI(TAG, "httpn task started");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(80);
    int s, r;
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);
	int use_buf = 0;
    int cs;
    while(1){
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");
         if(bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket bind failed errno=%d", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket bind done");
        if(listen (s, LISTENQ) != 0) {
            ESP_LOGE(TAG, "... socket listen failed errno=%d", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        while(1){
			use_buf = (use_buf+1) & 0x03;
            cs=accept(s,(struct sockaddr *)&remote_addr, &socklen);
			// --- set non blocking
            fcntl(cs,F_SETFL,O_NONBLOCK);
			vTaskDelay(10 / portTICK_PERIOD_MS);
			// --- fetch data
            do {
                bzero(recv_buf, sizeof(recv_buf));
                r = recv(cs, recv_buf, sizeof(recv_buf)-1,0);
				if (r > 0) {
					for(int i = 0; i < r; i++) {
						if (http.rxpos[use_buf] < (HTTP_RX_BUFSIZE-1)) {
							http.rx_buf[use_buf][http.rxpos[use_buf]] = recv_buf[i];
							http.rxpos[use_buf]++;
						}
					}
				}
            } while(r > 0);
			
			// --- decode http request
			http.rx_buf[use_buf][http.rxpos[use_buf]] = '\0';
			if ( (http.rxpos[use_buf] > 32) && (strstr(http.rx_buf[use_buf], "\r\n\r\n")) ) {
				// --- seperate 1st line for parsing
				char tmp_header[128];
				int tmp_len = (int)(strchr(http.rx_buf[use_buf], '\n') - &http.rx_buf[use_buf][0] - 1);
				strncpy(&tmp_header[0], &http.rx_buf[use_buf][0], tmp_len);
				tmp_header[tmp_len] = '\0';
				
				// --- prepare response header, 200 ok
				strcpy(http.tx_buf[use_buf], HTTP_HEADER);
				
				// --- decode http request
				bool is_good_cmd = false;
				bool in_progress = false;
				bool do_send = true;
				if (heat.op_in_progress != 1) {
					// -- get
					if (strstr(tmp_header, HTTP_GET)) {
						// --- cfg_get
						if (strstr(tmp_header, HTTP_GET_CFG)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Get config ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
							http.reply_get_cfg(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cfg_set
						if (strstr(tmp_header, HTTP_SET_CFG)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_set_cfg(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- results
						if (strstr(tmp_header, HTTP_GET_RESULTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_results(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- race info
						if (strstr(tmp_header, HTTP_GET_RACEINFO)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_raceinfo(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- session info
						if (strstr(tmp_header, HTTP_GET_SESSIONINFO)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_sessioninfo(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- pilot info
						if (strstr(tmp_header, HTTP_GET_PILOTINFO)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_pilotinfo(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- rssi
						if (strstr(tmp_header, HTTP_GET_RSSI)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_rssi(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- set trigger level
						if (strstr(tmp_header, HTTP_SET_TLEVEL)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_tlevel(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- get min max levels for calibration
						if (strstr(tmp_header, HTTP_GET_MINMAX)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_minmax(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- get exceptions
						if (strstr(tmp_header, HTTP_GET_EXCEPT)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_ex(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- get available events
						if (strstr(tmp_header, HTTP_GET_EVENTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_events(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- open event
						if (strstr(tmp_header, HTTP_GET_OPEN_EVENT)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_open_event(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- event pilots
						if (strstr(tmp_header, HTTP_GET_EVENT_PILOTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_event_pilots(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- close event
						if (strstr(tmp_header, HTTP_GET_CLOSE_EVENT)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_close_event(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- event results
						if (strstr(tmp_header, HTTP_GET_EVENT_RESULTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_event_results(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- get available sessions
						if (strstr(tmp_header, HTTP_GET_SESSIONS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_sessions(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- open session
						if (strstr(tmp_header, HTTP_GET_OPEN_SESSION)) {
							is_good_cmd = true;
							switch(heat.op_in_progress) {
								case 0:
									// - start open session in new thread
									strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
									http.reply_open_session(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
									is_good_cmd = false;
									in_progress = true;
									break;
								case 2:
									// - open session final, send answer
									heat.op_in_progress = 0;
									break;
							}
						}
						// --- get session pilots
						if (strstr(tmp_header, HTTP_GET_SESSION_PILOTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_get_session_pilots(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- close session
						if (strstr(tmp_header, HTTP_GET_CLOSE_SESSION)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_close_session(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- open heat
						if (strstr(tmp_header, HTTP_GET_OPEN_HEAT)) {
							is_good_cmd = true;
							switch(heat.op_in_progress) {
								case 0:
									// - start open heat in new thread
									strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
									http.reply_open_heat(&tmp_header[0], &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
									is_good_cmd = false;
									in_progress = true;
									break;
								case 2:
									// - open heat final, send answer
									heat.op_in_progress = 0;
									break;
							}
						}
						// --- cmd start
						if (strstr(tmp_header, HTTP_CMD_START)) {
							is_good_cmd = true;
							rt.start();
							rt.get_state();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Start ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd clear
						if (strstr(tmp_header, HTTP_CMD_CLEAR)) {
							is_good_cmd = true;
							// -- if event is open, commit cleared heat
							if (event.is_open) {
								// - do commit
								switch(heat.op_in_progress) {
								case 0:
									// - clear heat, but keep pilots
									rt.clear(true);
									// - start commit in new thread
									rt.state = 0;
									rt.set_state();
									ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_COMMIT, NULL, 0, portMAX_DELAY));
									is_good_cmd = false;
									in_progress = true;
									break;
								case 2:
									// - commit final, send answer
									heat.op_in_progress = 0;
									break;
								}
							} else {
								// - clear heat, but keep pilots
								rt.clear(true);
							}
							rt.get_state();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Clear ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd commit
						if (strstr(tmp_header, HTTP_CMD_COMMIT)) {
							is_good_cmd = true;
							switch(heat.op_in_progress) {
								case 0:
									// - start commit in new thread
									rt.state = 3;
									rt.set_state();
									ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_COMMIT, NULL, 0, portMAX_DELAY));
									is_good_cmd = false;
									in_progress = true;
									break;
								case 2:
									// - commit final, send answer
									heat.op_in_progress = 0;
									break;
							}
							rt.get_state();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Commit ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd stop
						if (strstr(tmp_header, HTTP_CMD_STOP)) {
							is_good_cmd = true;
							rt.stop();
							rt.get_state();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Stop ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd start calibration
						if (strstr(tmp_header, HTTP_CMD_START_CAL)) {
							is_good_cmd = true;
							rt.do_calib = true;
							rt.clear_minmax();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Start ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd save calibration
						if (strstr(tmp_header, HTTP_CMD_SAVE_CAL)) {
							is_good_cmd = true;
							rt.do_calib = false;
							rt.write_cal_data();
							cfg.nvs_set_cal();
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Save ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- cmd stop calibration
						if (strstr(tmp_header, HTTP_CMD_STOP_CAL)) {
							is_good_cmd = true;
							rt.do_calib = false;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							strcat(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], "Stop ");
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- file
						if ((!is_good_cmd) && (!in_progress)) {
							if (http.reply_file(&tmp_header[0], cs, use_buf)) {
								is_good_cmd = true;
								do_send = false;
							}
						}
					}
				} else {
					is_good_cmd = false;
					in_progress = true;
				}
				
				// --- send reply here?
				if (do_send) {
					// -- post
					if (strstr(tmp_header, HTTP_POST)) {
						// --- set exception ram
						if (strstr(tmp_header, HTTP_SET_EXCEPT)) {
							is_good_cmd = true;
							http.parse_ex(strstr(http.rx_buf[use_buf], "\r\n\r\n")+4);
							http.reply_done(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- new event
						if (strstr(tmp_header, HTTP_SET_NEW_EVENT)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_new_event(strstr(http.rx_buf[use_buf], "\r\n\r\n")+4, &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- set event pilots
						if (strstr(tmp_header, HTTP_SET_EVENT_PILOTS)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_set_event_pilots(strstr(http.rx_buf[use_buf], "\r\n\r\n")+4, &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
						// --- new session
						if (strstr(tmp_header, HTTP_GET_NEW_SESSION)) {
							is_good_cmd = true;
							strcpy(&http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])], HTTP_CONTENT_CSV);
							http.reply_new_session(strstr(http.rx_buf[use_buf], "\r\n\r\n")+4, &http.tx_buf[use_buf][strlen(http.tx_buf[use_buf])]);
						}
					}
					
					// -- 404
					if (!is_good_cmd) {
						if (in_progress) {
							// -- redo headers, 202 accepted
							strcpy(http.tx_buf[use_buf], HTTP_202);
							ESP_LOGI(TAG, "http 202: %s", tmp_header);
						} else {
							// -- redo headers, 404 not found
							strcpy(http.tx_buf[use_buf], HTTP_404);
							ESP_LOGI(TAG, "http 404: %s", tmp_header);
						}
					} else {
						ESP_LOGI(TAG, "%s", tmp_header);
					}
				
					// -- write response
					int sent_size = 0;
					int send_size = strlen(http.tx_buf[use_buf]);
					char * sbuf = &http.tx_buf[use_buf][0];
					int cnt_err = 0;
					do {
						sent_size = send(cs, sbuf, send_size, 0);
						if (sent_size > 0) {
							send_size -= sent_size;
							sbuf += sent_size;
							cnt_err = 0;
						} else {
							vTaskDelay(10 / portTICK_PERIOD_MS);
							if (cnt_err < 80) {
								cnt_err++;
							} else {
								// - too many fails, cut it off
								send_size = 0;
								ESP_LOGE(TAG, "... Send failed");
							}
						}
					} while(send_size > 0);
				}
				
				// --- clear rx buffer and pos
				http.rxpos[use_buf] = 0;
				bzero(http.rx_buf, sizeof(http.rx_buf));
			}
			shutdown(cs, 0);
            close(cs);
        }
        ESP_LOGI(TAG, "... server will be opened in 5 seconds");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "...tcp_client task closed");
}
