#include "main.h"

// ****** init Session
void oo_Session::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init session");
	
	// --- some vars to default
	// -- session
	nr = 0;
	mode = 0;
	sprintf(name, "   ");
	heat_cnt = 0;
	for (uint8_t i=0;i<4;i++) {
		st_heat_empty.pilots_nr[i] = 0xffff;
	}
	is_open = false;
	mod_cnt = 0;
}

// ****** open Session
void oo_Session::open(uint8_t snr) {
	char ctmp[128];
	// --- set session stuff
	mod_cnt++;
	heat.mod_cnt++;
	is_open = true;
	nr = snr;
	for(uint8_t i=0;i<16;i++) heats[i] = st_heat_empty;
	// --- read session parameters, open session config file
	sprintf(ctmp, "/data/%s/session%d/session.txt", event.name, nr);
	sd.cfg_file_open(ctmp, "r");
	if (sd.cfg_file != NULL) {
		do {
			sd.cfg_file_getparm();
			// -- mode
			if (strcmp(sd.cfg_parm, CFG_SESSION_MODE) == 0) {
				mode = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
			}
			// -- name
			if (strcmp(sd.cfg_parm, CFG_SESSION_NAME) == 0) {
				strncpy(name, sd.cfg_value, 40);
			}
		} while(sd.cfg_parm[0] != 0xff);
		printf("Session %d '%s' mode %d\r\n", nr, name, mode);
		// -- close config file
		sd.cfg_file_close();
	}
	// --- read session pilots
	read_session_pilots();
	
	// --- read heats state
	sprintf(ctmp, "%s/session%d/state.csv", event.name, nr);
	sd.data_file_open(ctmp, "r");
	if (sd.data_file != NULL) {
		printf("csv file opened for reading '%s'\r\n", ctmp);
		while (sd.data_file_getline_csv()) {
			heats[sd.csv_array[0]].state = sd.csv_array[1];
			heats[sd.csv_array[0]].pilots_state[sd.csv_array[2]] = sd.csv_array[3];
			printf("Heat Nr '%d' state '%d' pilot '%d' state '%d'\r\n", sd.csv_array[0], heats[sd.csv_array[0]].state, sd.csv_array[2], heats[sd.csv_array[0]].pilots_state[sd.csv_array[2]]);
		}
		sd.data_file_close();
	} else {
		printf("error opening csv file for reading '%s'\r\n", ctmp);
	}
	
	// --- read session results
	sprintf(ctmp, "%s/session%d/results.csv", event.name, nr);
	sd.data_file_open(ctmp, "r");
	if (sd.data_file != NULL) {
		printf("csv file opened for reading '%s'\r\n", ctmp);
		while (sd.data_file_getline_csv()) {
			session.heats[sd.csv_array[0]].nr = sd.csv_array[0];
			session.heats[sd.csv_array[0]].lapcount[sd.csv_array[1]] = sd.csv_array[2];
			session.heats[sd.csv_array[0]].heat_time[sd.csv_array[1]] = sd.csv_array[3];
			session.heats[sd.csv_array[0]].fastest_laps_time[sd.csv_array[1]] = sd.csv_array[4];
			printf("Heat Nr '%d' chn '%d' laps '%d' heat time '%d' fastest lap '%d'\r\n", session.heats[sd.csv_array[0]].nr,
																		sd.csv_array[1],
																		session.heats[sd.csv_array[0]].lapcount[sd.csv_array[1]],
																		session.heats[sd.csv_array[0]].heat_time[sd.csv_array[1]],
																		session.heats[sd.csv_array[0]].fastest_laps_time[sd.csv_array[1]]);
		}
		sd.data_file_close();
	}
		//for(uint8_t h=0;h<heat_cnt;h++) {
		//	session.heats[h].nr = h;
		//}
	
	// --- collect results from runs
	//collect_results();
	
	// --- open heat 0, first heat
	heat.current.nr = 0;
	ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_OPEN, NULL, 0, portMAX_DELAY));
	//heat.open();
}

// ****** create new Session
void oo_Session::create_new(uint8_t sess_mode, uint8_t gen_mode) {
	printf("create new session sess_mode '%d' gen_mode '%d'\r\n", sess_mode, gen_mode);
	char ctmp[256];
	// --- build name of new dir and create directory
	sprintf(ctmp, "/RTsd/data/%s/session%d", event.name, event.sessions_cnt);
	printf("create dir for session '%s'\r\n", ctmp);
	sd.dir_create(&ctmp[0]);
	
	// --- write session config file
	// -- create file
	sprintf(ctmp, "/data/%s/session%d/session.txt", event.name, event.sessions_cnt);
	sd.cfg_file_open(ctmp, "w");
	if (sd.cfg_file != NULL) {
		// -- write config data
		// - mode
		sprintf(sd.data_line, "mode=%d;\r\n", sess_mode);
		sd.cfg_file_writeline();
		// - name
		switch(sess_mode) {
			// training
			case 0:
				sprintf(sd.data_line, "name=Training %d;\r\n", event.cnt_sess_type[0]+1);
				event.cnt_sess_type[0]++;
				break;
			// quali
			case 1:
				sprintf(sd.data_line, "name=Qualifying %d;\r\n", event.cnt_sess_type[1]+1);
				event.cnt_sess_type[1]++;
				break;
			// race
			case 2:
				sprintf(sd.data_line, "name=Race %d;\r\n", event.cnt_sess_type[2]+1);
				event.cnt_sess_type[2]++;
				break;
		}
		sd.cfg_file_writeline();
		// -- close file
		sd.cfg_file_close();
	}
	
	// --- calc number of heats
	uint8_t hcount = (event.pilots_cnt + 3) / 4;
	
	// --- write state csv file
	// -- create file
	sprintf(ctmp, "%s/session%d/state.csv", event.name, event.sessions_cnt);
	printf("%s\r\n", ctmp);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		// -- write state lines
		printf("Session create new hcount '%d'\r\n", hcount);
		for(uint8_t i=0;i<hcount;i++) {
			for(uint8_t c=0;c<4;c++) {
				sprintf(sd.data_line, "%x;0;%x;0;\r\n", i, c);
				printf("%s", sd.data_line);
				sd.data_file_writeline();
			}
		}
		// -- close file
		sd.data_file_close();
	}
	
	// --- create session results
	// -- create file
	sprintf(ctmp, "%s/session%d/results.csv", event.name, event.sessions_cnt);
	printf("%s\r\n", ctmp);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		// -- write empty result lines
		for(uint8_t i=0;i<hcount;i++) {
			for(uint8_t c=0;c<4;c++) {
				sprintf(sd.data_line, "%x;%x;0;00000000;00000000;\r\n", i, c);
				printf("%s", sd.data_line);
				sd.data_file_writeline();
			}
		}
		// -- close file
		sd.data_file_close();
	}
	
	// --- generate pilot field
	// -- create file
	sprintf(ctmp, "%s/session%d/pilots.csv", event.name, event.sessions_cnt);
	printf("%s\r\n", ctmp);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		// -- generate pilot field
		gen_field(gen_mode, hcount);
		// -- close file
		sd.data_file_close();
	}
	
	// --- touch runs directories
	for(uint8_t i=0;i<hcount;i++) {
		// -- build name of new dir and create directory
		sprintf(ctmp, "/RTsd/data/%s/session%d/run%d", event.name, event.sessions_cnt, i);
		printf("create dir for run '%s'\r\n", ctmp);
		sd.dir_create(&ctmp[0]);
	}
	
	// --- increment event session counter
	event.sessions_cnt++;
}

// ****** generate pilot field for session
void oo_Session::gen_field(uint8_t gen_mode, uint8_t hcount) {
	uint8_t pitmp = 0;
	
	// --- write pilot lines, all pilots in event
	if (gen_mode == 0) {
		for(uint8_t i=0;i<hcount;i++) {
			for(uint8_t c=0;c<4;c++) {
				pitmp = (i*4)+c;
				if (pitmp < event.pilots_cnt) {
					sprintf(sd.data_line, "%x;%x;%x;\r\n", i, c, event.pilots[pitmp]);
					printf("%s", sd.data_line);
					sd.data_file_writeline();
				}
			}
		}
	}
	
	// --- quali results
	if (gen_mode == 1) {
		// -- pilots with quali time
		for(uint8_t i=0;i<hcount;i++) {
			for(uint8_t c=0;c<4;c++) {
				pitmp = (i*4)+c;
				if (pitmp < event.pilots_cnt) {
					sprintf(sd.data_line, "%x;%x;%x;\r\n", i, c, event.quali_time[pitmp].pilot_nr);
					printf("%s", sd.data_line);
					sd.data_file_writeline();
				}
			}
		}
	}
}

// ****** close Session
void oo_Session::close(void) {
	// --- close
	is_open = false;
	heat.is_open = false;
	// --- handle mod counter
	mod_cnt++;
	// --- clear session
	heat_cnt = 0;
	for (uint8_t i=0;i<16;i++) {
		heats[i] = st_heat_empty;
	}
	heat.clear();
}

// ****** read session pilots from session directory
void oo_Session::read_session_pilots(void) {
	char ctmp[256];
	//char * ptmp = &sd.data_line[0];
	//uint8_t etmp = 0;
	
	// --- clear pilots array
	for(uint8_t i=0;i<16;i++) {
		heats[i].pilots_nr[0] = 0xffff;
		heats[i].pilots_nr[1] = 0xffff;
		heats[i].pilots_nr[2] = 0xffff;
		heats[i].pilots_nr[3] = 0xffff;
	}
	
	// --- open session pilots file
	sprintf(ctmp, "%s/session%d/pilots.csv", event.name, nr);
	sd.data_file_open(ctmp, "r");
	if (sd.data_file != NULL) {
		// --- read by line
		heat_cnt = 0;
		while (sd.data_file_getline_csv()) {
			//pilots[sd.csv_array[0]][sd.csv_array[1]] = (uint8_t)sd.csv_array[2];
			heats[sd.csv_array[0]].pilots_nr[sd.csv_array[1]] = (uint16_t)sd.csv_array[2];
			printf("%d;%d;%d\r\n", sd.csv_array[0], sd.csv_array[1], sd.csv_array[2]);
			if (heat_cnt < sd.csv_array[0]) heat_cnt = (uint16_t)sd.csv_array[0];
		}
		heat_cnt++;
		printf("parsed heat count '%d'\r\n", heat_cnt);
		
		// --- close event pilots data file
		sd.data_file_close();
	}
}
