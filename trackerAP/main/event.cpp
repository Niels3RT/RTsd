#include "main.h"

// ****** init Event
void oo_Event::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init event");
	
	// --- some vars to default
	// -- event
	nr = 1;
	sprintf(name, "---");
	// -- empty event
	event_empty.quali_mode = 0;
	event_empty.quali_laps = 3;
	event_empty.quali_otime = 120;
	event_empty.race_mode = 0;
	event_empty.race_laps = 3;
	current = event_empty;
	// -- empty result
	result_empty.pilot_nr = 0xffff;
	result_empty.laps = 0;
	result_empty.time = 0xffffffff;
	cnt_sess_type[0] = 0;
	cnt_sess_type[1] = 0;
	cnt_sess_type[2] = 0;
	clear_results();
	// -- pilots
	pilots_cnt = 0;
	pilots_all_cnt = 4;
	pilots_data[0].nr = 0;
	strcpy(pilots_data[0].name, DEFAULT_PILOT_1);
	pilots_data[1].nr = 1;
	strcpy(pilots_data[1].name, DEFAULT_PILOT_2);
	pilots_data[2].nr = 2;
	strcpy(pilots_data[2].name, DEFAULT_PILOT_3);
	pilots_data[3].nr = 3;
	strcpy(pilots_data[3].name, DEFAULT_PILOT_4);
	is_open = false;
	mod_cnt = 0;
	// -- sessions
	sessions_cnt = 0;
	
	// --- read pilots from csv
	read_pilots();
}

// ****** open event
void oo_Event::open(uint8_t nr) {
	is_open = true;
	uint8_t cnt = 0;
	mod_cnt++;
	session.mod_cnt++;
	heat.mod_cnt++;
	char ctmp[128];
	// --- open event directory
	if (sd.dir_open(HTTP_SD_PATH_EVENTS)) {
		// -- read dir entries
		sd.dir_get_entry();
		while(sd.entry) {
			if (sd.entry->d_type == DT_DIR) {
				if (cnt == nr) {
					strncpy(name, sd.entry->d_name, 39);
					//printf("Path to event '%s'\r\n", name);
				}
				cnt++;
			}
			sd.dir_get_entry();
		}
		// -- close directory
		sd.dir_close();
		// -- read event pilots
		read_event_pilots();
		// -- read event parameters, open event config file
		sprintf(ctmp, "/data/%s/event.txt", name);
		sd.cfg_file_open(ctmp, "r");
		if (sd.cfg_file != NULL) {
			do {
				sd.cfg_file_getparm();
				// -- don't work on comments or eof
				if ((sd.cfg_parm[0] != '#') && (sd.cfg_parm[0] != 0xff)){
					//printf("%s : %s\r\n", sd.cfg_parm, sd.cfg_value);
					// -- quali mode
					if (strcmp(sd.cfg_parm, CFG_QUALI_MODE) == 0) {
						quali_mode = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
						printf("quali mode from txt '%d'\r\n", quali_mode);
					}
					// -- quali laps
					if (strcmp(sd.cfg_parm, CFG_QUALI_LAPS) == 0) {
						quali_laps = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
						printf("quali laps from txt '%d'\r\n", quali_laps);
					}
					// -- quali overtime
					if (strcmp(sd.cfg_parm, CFG_QUALI_OTIME) == 0) {
						quali_otime = buf.buf2uintX_t_dec(&sd.cfg_value[0]);
						printf("quali overtime from txt '%d'\r\n", quali_otime);
					}
					// -- race mode
					if (strcmp(sd.cfg_parm, CFG_RACE_MODE) == 0) {
						race_mode = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
						printf("race mode from txt '%d'\r\n", race_mode);
					}
					// -- race laps
					if (strcmp(sd.cfg_parm, CFG_RACE_LAPS) == 0) {
						race_laps = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
						printf("race laps from txt '%d'\r\n", race_laps);
					}
				}
			} while(sd.cfg_parm[0] != 0xff);
			// -- close config file
			sd.cfg_file_close();
			// -- fetch sessions
			sessions_cnt = 0;
			sprintf(ctmp, "/RTsd/data/%s", name);
			if (sd.dir_open(ctmp)) {
				// -- read dir entries
				sd.dir_get_entry();
				while(sd.entry) {
					if ((sd.entry->d_type == DT_DIR) && (sessions_cnt < 32)) {
						//strncpy(sessions[sessions_cnt].name, sd.entry->d_name, 39);
						// -- nr
						sessions[sessions_cnt].nr = sessions_cnt;
						// -- session info from config file
						strncpy(sessions[sessions_cnt].name, sd.entry->d_name, 40);
						sprintf(ctmp, "/data/%s/%s/session.txt", name, sessions[sessions_cnt].name);
						sd.cfg_file_open(ctmp, "r");
						if (sd.cfg_file != NULL) {
							do {
								sd.cfg_file_getparm();
								// -- mode training
								if (strcmp(sd.cfg_parm, CFG_SESSION_MODE) == 0) {
									sessions[sessions_cnt].mode = (uint8_t)buf.buf2uintX_t_dec(&sd.cfg_value[0]);
									//printf("session mode from txt '%d'\r\n", sessions[sessions_cnt].mode);
									cnt_sess_type[sessions[sessions_cnt].mode]++;
									printf("'%s' - '%d'\r\n", ctmp, cnt_sess_type[sessions[sessions_cnt].mode]);
								}
								// -- name
								if (strcmp(sd.cfg_parm, CFG_SESSION_NAME) == 0) {
									strncpy(sessions[sessions_cnt].name, sd.cfg_value, 40);
								}
							} while(sd.cfg_parm[0] != 0xff);
						}
						printf("\r\nSession '%d' '%s' mode '%d'\r\n", sessions[sessions_cnt].nr, sessions[sessions_cnt].name, sessions[sessions_cnt].mode);
						sd.cfg_file_close();
						sessions_cnt++;
					}
					sd.dir_get_entry();
				}
			}
			// -- close dir
			sd.dir_close();
		} else {
			ESP_LOGE(TAG, "Failed to open event config file :(");
		}
	}
	
	// --- clear event results
	clear_results();
	
	// --- read evént results
	// - open file
	sprintf(ctmp, "%s/results.csv", event.name);
	sd.data_file_open(ctmp, "r");
	uint8_t cnt_f = 0;
	uint8_t cnt_q = 0;
	if (sd.data_file != NULL) {
		while(sd.data_file_getline_csv()) {
			switch(sd.csv_array[0]) {
				// - fastest laps
				case 0:
					fastest_laps[cnt_f].pilot_nr = sd.csv_array[1];
					fastest_laps[cnt_f].laps = 1;
					fastest_laps[cnt_f].time = sd.csv_array[2];
					cnt_f++;
					break;
				// - quali results
				case 1:
					quali_time[cnt_q].pilot_nr = sd.csv_array[1];
					quali_time[cnt_q].laps = sd.csv_array[2];
					quali_time[cnt_q].time = sd.csv_array[3];
					cnt_q++;
					break;
			}
		}
		// -- close data file
		sd.data_file_close();
	}
	// --- ****** debug: print fastest laps array
	for(uint8_t i=0;i<CFG_MAX_PILOTS;i++) {
		if (fastest_laps[i].pilot_nr != 0xffff) {
			printf("fastest laps pilot '%02d' fastest lap '%05d'\r\n", fastest_laps[i].pilot_nr, fastest_laps[i].time);
		}
	}
	// --- ****** debug: print quali array
	for(uint8_t i=0;i<CFG_MAX_PILOTS;i++) {
		if (quali_time[i].pilot_nr != 0xffff) {
			printf("quali pilot '%02d' laps '%02d' time '%06d'\r\n", quali_time[i].pilot_nr, quali_time[i].laps, quali_time[i].time);
		}
	}
}

// ****** close event
void oo_Event::close(void) {
	is_open = false;
	clear_results();
	mod_cnt++;
	session.mod_cnt++;
	heat.mod_cnt++;
}

// ****** create new event
void oo_Event::create_new(st_event * evtmp) {
	char ctmp[256];
	// --- build name of new dir and create directory
	sprintf(ctmp, "/RTsd/data/%s", evtmp->name);
	printf("create dir for event '%s'\r\n", ctmp);
	sd.dir_create(&ctmp[0]);
	
	// --- open new event.txt
	sprintf(ctmp, "/data/%s/event.txt", evtmp->name);
	sd.cfg_file_open(ctmp, "w");
	
	// --- write config data
	sprintf(sd.data_line, "quali_mode=%d;\r\n", evtmp->quali_mode);
	sd.cfg_file_writeline();
	sprintf(sd.data_line, "quali_laps=%d;\r\n", evtmp->quali_laps);
	sd.cfg_file_writeline();
	sprintf(sd.data_line, "quali_otime=%d;\r\n", evtmp->quali_otime);
	sd.cfg_file_writeline();
	sprintf(sd.data_line, "race_mode=%d;\r\n", evtmp->race_mode);
	sd.cfg_file_writeline();
	sprintf(sd.data_line, "race_laps=%d;\r\n", evtmp->race_laps);
	sd.cfg_file_writeline();
	
	// --- close config file
	sd.cfg_file_close();
	
	// --- touch new events pilots.csv
	sprintf(ctmp, "%s/pilots.csv", evtmp->name);
	sd.data_file_open(ctmp, "w");
	sd.data_file_close();
}

// ****** read event pilots from event directory
void oo_Event::read_event_pilots(void) {
	char ctmp[256];
	
	// --- open event pilots file
	sprintf(ctmp, "%s/pilots.csv", name);
	sd.data_file_open(ctmp, "r");
	
	// --- read by line
	pilots_cnt = 0;
	while (sd.data_file_getline_csv()) {
		pilots[pilots_cnt] = (uint8_t)sd.csv_array[0];
		pilots_cnt++;
	}
	
	// --- close event pilots data file
	sd.data_file_close();
}

// ****** write event pilots to event directory
void oo_Event::write_event_pilots(void) {
	char ctmp[256];
	
	// --- open event pilots file
	sprintf(ctmp, "%s/pilots.csv", name);
	sd.data_file_open(ctmp, "w");
	
	// --- write by line
	for (uint8_t i=0;i<pilots_cnt;i++) {
		sprintf(sd.data_line, "%x;\r\n", pilots[i]);
		sd.data_file_writeline();
	}
	
	// --- close event pilots data file
	sd.data_file_close();
}

// ****** read all pilots from sd card
void oo_Event::read_pilots(void) {
	char * pos = &sd.data_line[0];
	uint8_t len = 0;
	//char tname[40];
	pilots_all_cnt = 0;

	// --- open pilots data file
	sd.data_file_open("pilots.csv", "r");
	
	// --- walk through lines in data file
	while(sd.data_file_getline()) {
		// -- is valid data line?
		if (strlen(sd.data_line) > 3) {
			// - find ';' for pos and len
			pos = strchr(&sd.data_line[0], ';') + 1;
			// - decode pilot number
			uint8_t cnt = 0;
			uint16_t nr = 0;
			while ((sd.data_line[cnt] >= 0x30) && (sd.data_line[cnt] <= 0x39)) {
				nr *= 10;
				nr += sd.data_line[cnt] - 0x30;
				cnt++;
			}
			pilots_data[pilots_all_cnt].nr = nr;
			// - get pilot name
			len = strchr(pos, ';') - pos;
			if (len > 40) len = 40;
			strncpy(pilots_data[pilots_all_cnt].name, pos, len);
			pilots_data[pilots_all_cnt].name[len] = '\0';
			
			printf("nr '%d' len '%d' name '%s'\r\n", pilots_data[pilots_all_cnt].nr, len, pilots_data[pilots_all_cnt].name);
			
			// - increment pilot counter
			pilots_all_cnt++;
		}
	}
	
	// --- close pilots data file
	sd.data_file_close();
}

// ****** collect results from runs
void oo_Event::collect_results(void) {
	char ctmp[256];
	uint8_t pcount = 0;
	printf("----------------------------------------------------------------------\r\n");
	printf("Yeah, collect session results!\r\n");
	// --- clear event results
	clear_results();
	// --- walk through sessions
	for(uint8_t s=0;s<sessions_cnt;s++) {
		printf("Session '%d' mode '%d'\r\n", s, sessions[s].mode);
		// -- quali times
		if (sessions[s].mode == 1) {
			// -- read session pilots
			pcount = 0;
			// - open file
			sprintf(ctmp, "%s/session%d/pilots.csv", event.name, s);
			sd.data_file_open(ctmp, "r");
			// - read by line
			while (sd.data_file_getline_csv()) {
				fastest_laps[(sd.csv_array[0]*4)+sd.csv_array[1]].pilot_nr = sd.csv_array[2];
				quali_time[(sd.csv_array[0]*4)+sd.csv_array[1]].pilot_nr = sd.csv_array[2];
				pcount++;
			}
			// - close session pilots data file
			sd.data_file_close();
			// -- fetch results for pilots
			// - open file
			sprintf(ctmp, "%s/session%d/results.csv", event.name, s);
			sd.data_file_open(ctmp, "r");
			// - read by line
			while (sd.data_file_getline_csv()) {
				uint8_t pcount_tmp = (sd.csv_array[0]*4)+sd.csv_array[1];
				if (pcount_tmp < pcount) {
					// - fastest laps
					if (fastest_laps[pcount_tmp].time > sd.csv_array[4]) {
						printf("lap is faster! '%d'\r\n", s);
						fastest_laps[pcount_tmp].time = sd.csv_array[4];
					}
					fastest_laps[pcount_tmp].laps = 1;
					// - quali laps + overtime
					if ((quali_time[pcount_tmp].laps < sd.csv_array[2])																// more laps flown
						|| ((quali_time[pcount_tmp].laps == sd.csv_array[2]) && (quali_time[pcount_tmp].time > sd.csv_array[3]))) {	// same lapcount, but faster time
							printf("quali is better! '%d'\r\n", s);
							quali_time[pcount_tmp].laps = sd.csv_array[2];
							quali_time[pcount_tmp].time = sd.csv_array[3];
					}
				}
			}
			// - close session pilots data file
			sd.data_file_close();
		}
	}
	// --- sort fastest laps array
	sort_fastest_laps(pcount);
	// --- sort quali array
	sort_quali(pcount);
	// --- ****** debug: print fastest laps array
	for(uint8_t i=0;i<32;i++) {
		if (fastest_laps[i].pilot_nr != 0xffff) {
			printf("fastest laps pilot '%02d' fastest lap '%05d'\r\n", fastest_laps[i].pilot_nr, fastest_laps[i].time);
		}
	}
	// --- ****** debug: print quali array
	for(uint8_t i=0;i<32;i++) {
		if (quali_time[i].pilot_nr != 0xffff) {
			printf("quali pilot '%02d' laps '%02d' time '%06d'\r\n", quali_time[i].pilot_nr, quali_time[i].laps, quali_time[i].time);
		}
	}
	
	// --- write event result to file
	// - open file
	sprintf(ctmp, "%s/results.csv", event.name);
	sd.data_file_open(ctmp, "w");
	// - write fastest laps
	for (uint8_t i=0;i<pcount;i++) {
		sprintf(sd.data_line, "0;%x;%08x;\r\n", fastest_laps[i].pilot_nr, fastest_laps[i].time);
		sd.data_file_writeline();
		printf("%s", sd.data_line);
	}
	// - write quali results
	for (uint8_t i=0;i<pcount;i++) {
		sprintf(sd.data_line, "1;%x;%x;%08x;\r\n", quali_time[i].pilot_nr, quali_time[i].laps, quali_time[i].time);
		sd.data_file_writeline();
		printf("%s", sd.data_line);
	}
	// - close session pilots data file
	sd.data_file_close();
	
	printf("Yeah, collect session results finished!\r\n");
	printf("----------------------------------------------------------------------\r\n");
}

// ****** sort fastest laps array
void oo_Event::sort_fastest_laps(uint8_t pcount) {
	st_result r_tmp = result_empty;
	// --- sort array
	for(uint8_t i=0;i<pcount;i++) {
		for(uint8_t p=0;p<(pcount-1);p++) {
			if ((quali_time[p].laps < quali_time[p+1].laps)																	// more laps flown
						|| ((quali_time[p].laps == quali_time[p+1].laps) && (quali_time[p].time > quali_time[p+1].time))) {	// same lapcount, but faster time
				r_tmp = quali_time[p];
				quali_time[p] = quali_time[p+1];
				quali_time[p+1] = r_tmp;
			}
		}
	}
}

// ****** sort quali array
void oo_Event::sort_quali(uint8_t pcount) {
	st_result r_tmp = result_empty;
	// --- sort array
	for(uint8_t i=0;i<pcount;i++) {
		for(uint8_t p=0;p<(pcount-1);p++) {
			if (fastest_laps[p].time > fastest_laps[p+1].time) {
				r_tmp = fastest_laps[p];
				fastest_laps[p] = fastest_laps[p+1];
				fastest_laps[p+1] = r_tmp;
			}
		}
	}
}

// ****** clear event results (fastest laps, quali)
void oo_Event::clear_results(void) {
	for(uint8_t i=0;i<CFG_MAX_PILOTS;i++) {
		fastest_laps[i] = result_empty;
		quali_time[i] = result_empty;
	}
}
