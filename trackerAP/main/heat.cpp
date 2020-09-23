#include "main.h"

// ****** init Heat
void oo_Heat::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init heat");
	
	// --- some vars to default
	// -- heat
	current.nr = 1;
	sprintf(current.name, "0/0");
	for (uint8_t i=0;i<rt.max_chn;i++) current.pilots_nr[i] = i;
	is_open = false;
	mod_cnt = 0;
	op_in_progress = 0;
}

// ****** open heat
void oo_Heat::open(void) {
	char ctmp[256];
	char trssi[8];
	uint8_t nrtmp = current.nr;
	if (nrtmp >= session.heat_cnt) nrtmp = session.heat_cnt - 1;
	// --- clear heat stuff in tracker
	rt.clear(false);
	// --- set in progress flag
	op_in_progress = 1;
	// --- set current heat and make active
	is_open = true;
	mod_cnt++;
	current = session.heats[nrtmp];
	current.nr = nrtmp;
	sprintf(current.name, "%d / %d", heat.current.nr + 1, session.heat_cnt);
	printf("HeatNr '%d'\r\n", current.nr);
	// --- copy rssi from file to fpga sdram
	rt.count = 0;
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/rssi.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "r");
	if (sd.data_file != NULL) {
		printf("csv file opened for reading '%s'\r\n", ctmp);
		while(sd.data_file_getline_csv()) {
			if (rt.count < sd.csv_array[0]) rt.count = sd.csv_array[0];
			trssi[0] = (sd.csv_array[1] >> 8) & 0xff;				// word 0
			trssi[1] = sd.csv_array[1] & 0xff;
			trssi[2] = (sd.csv_array[2] >> 8) & 0xff;				// word 1
			trssi[3] = sd.csv_array[2] & 0xff;
			trssi[4] = (sd.csv_array[3] >> 8) & 0xff;				// word 2
			trssi[5] = sd.csv_array[3] & 0xff;
			trssi[6] = (sd.csv_array[4] >> 8) & 0xff;				// word 3
			trssi[7] = sd.csv_array[4] & 0xff;
			rtspi.write64(&trssi[0]);								// write 64bit word to transfer register
			rtspi.transmit24(RT_REG2SDRAM, sd.csv_array[0], 0);		// set sdram address (rssi block nr)
		}
		// -- close data file
		sd.data_file_close();
	}
	// --- read trigger levels from results data file
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/results.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "r");
	if (sd.data_file != NULL) {
		// -- walk through lines in file
		while(sd.data_file_getline_csv()) {
			//- find line with trigger levels
			if (sd.csv_array[0] == 0) {
				while(!rt.pd_isready());
				// set trigger level mode to fixed
				rt.pd_set_fixed_mode(0x0f);
				// write trigger levels to rt
				for (uint8_t i=0;i<rt.max_chn;i++) rt.pd_set_tlevel(i, sd.csv_array[i+1]);
			}
		}
		// -- close data file
		sd.data_file_close();
	} else {
		// -- set trigger levels to default and write to rt
		for (uint8_t i=0;i<rt.max_chn;i++) rt.pd_set_tlevel(i, rt.det_auto_min);
	}
	// --- set rt state to current heat state
	rt.state = current.state;
	rt.set_state();
	// --- set rt count to max from rssi file
	rt.set_count();
	
	// --- load exceptions
	load_exceptions();
	
	// --- start peak detect in rt
	rt.pd_start();
	
	// --- remove in progress flag
	op_in_progress = 2;
}

// ****** clear heat
void oo_Heat::clear(void) {
	// --- clear current heat
	rt.clear(false);
	// --- handle mod counter
	mod_cnt++;
}

// ****** commit heat
void oo_Heat::commit(void) {
	char ctmp[256];
	// --- increment session mod count
	session.mod_cnt++;
	// --- set in progress flag
	op_in_progress = 1;
	// --- write current heat data to session heats array
	current.state = rt.state;
	session.heats[current.nr] = current;
	// --- limit lapcount in quali consecutive lap mode
	if ((session.mode == SESSION_MODE_QUALI) && (event.quali_mode == QUALI_CONSEC_LAPS)) {
		// -- walk through channels
		for(uint8_t i=0;i<rt.max_chn;i++) {
			// - more laps flown than consecutive limit? Position time is already best n laps.
			if (session.heats[current.nr].lapcount[i] > event.quali_laps) {
				session.heats[current.nr].lapcount[i] = event.quali_laps;
			}
		}
	}
	printf("1 current.state '%x' rt.state '%x'\r\n", current.state, rt.state);
	// --- save results
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/results.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		printf("csv file opened for writing '%s'\r\n", ctmp);
		// -- trigger levels
		sprintf(sd.data_line, "0;%04x;%04x;%04x;%04x;\r\n", rt.trg_level[0], rt.trg_level[1], rt.trg_level[2], rt.trg_level[3]);
		sd.data_file_writeline();
		// -- walk through channels
		for (uint8_t c=0;c<rt.max_chn;c++) {
			// - hits
			for (uint8_t i=0;i<current.lapcount[c];i++) {
				sprintf(sd.data_line, "1;%x;%02x;%08x;\r\n", c, i, laps[c][i]);
				sd.data_file_writeline();
			}
		}
		// -- fastest laps, order by fastest
		for (uint8_t i=0;i<rt.max_chn;i++) {
			sprintf(sd.data_line, "2;%x;%02x;%08x;\r\n", i, heat.current.pos_fastest_lap[i], laps[i][heat.current.fastest_laps_lapnr[heat.current.pos_fastest_lap[i]]]);
			sd.data_file_writeline();
		}
		// -- position in heat
		for (uint8_t i=0;i<rt.max_chn;i++) {
			sprintf(sd.data_line, "3;%x;%02x;%02x;%08x;\r\n", i, heat.current.pos_nr[i], heat.current.lapcount[heat.current.pos_nr[i]], heat.current.heat_time[heat.current.pos_nr[i]]);
			sd.data_file_writeline();
		}
		// -- close data file
		sd.data_file_close();
	}
	printf("2 current.state '%x'\r\n", current.state);
	// --- save rssi
	char trssi[16];
	uint16_t rtmp[8];
	for (uint8_t i=0;i<rt.max_chn;i++) rtmp[i] = 0;
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/rssi.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		printf("csv file opened for writing '%s'\r\n", ctmp);
		for (uint16_t i=0;i<rt.count;i++) {
			// -- read line (4 words a 16bits) of rssi from sdram
			rtspi.transmit24(RT_SDRAM2REG, i, 0);			// set sdram address (rssi block nr)
			rtspi.read64(&trssi[0]);						// fetch 64bit word from transfer register
			// -- write line of rssi to file
			for (uint8_t i=0;i<rt.max_chn;i++) rtmp[i] = ((trssi[i]<<8) + trssi[i+1]) & 0xfff;
			if (rt.max_chn == 4) {
				sprintf(sd.data_line, "%06x;%03x;%03x;%03x;%03x;\r\n", i, rtmp[0], rtmp[1], rtmp[2], rtmp[3]);
			}
			if (rt.max_chn == 8) {
				sprintf(sd.data_line, "%06x;%03x;%03x;%03x;%03x;%03x;%03x;%03x;%03x;\r\n", i, rtmp[0], rtmp[1], rtmp[2], rtmp[3], rtmp[4], rtmp[5], rtmp[6], rtmp[7]);
			}
			sd.data_file_writeline();
		}
		// -- close data file
		sd.data_file_close();
	} else {
		printf("error opening csv file for writing '%s'\r\n", ctmp);
	}
	
	// --- save heats states
	sprintf(ctmp, "/RTsd/data/%s/session%d/state.csv", event.name, session.nr);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		printf("csv file opened for writing '%s'\r\n", ctmp);
		for (uint8_t i=0;i<session.heat_cnt;i++) {
			for (uint8_t c=0;c<rt.max_chn;c++) {
				sprintf(sd.data_line, "%x;%x;%x;%x;\r\n", i, session.heats[i].state, c, session.heats[i].pilots_state[c]);
				sd.data_file_writeline();
			}
		}
		sd.data_file_close();
	} else {
		printf("error opening csv file for reading '%s'\r\n", ctmp);
	}
	
	// --- save exceptions
	save_exceptions();
	
	// --- save session results
	sprintf(ctmp, "/RTsd/data/%s/session%d/results.csv", event.name, session.nr);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		printf("csv file opened for writing '%s'\r\n", ctmp);
		for (uint8_t h=0;h<session.heat_cnt;h++) {
			for (uint8_t c=0;c<rt.max_chn;c++) {
				sprintf(sd.data_line, "%x;%x;%x;%08x;%08x;\r\n", session.heats[h].nr,
											c,
											session.heats[h].lapcount[c],
											session.heats[h].heat_time[c],
											session.heats[h].fastest_laps_time[c]);
				sd.data_file_writeline();
				printf("%s", sd.data_line);
			}
		}
		sd.data_file_close();
	} else {
		printf("error opening csv file for reading '%s'\r\n", ctmp);
	}
	
	// --- recalc event results
	event.collect_results();
	
	// --- remove in progress flag
	op_in_progress = 2;
	
	printf("Commit HeatNr '%d'\r\n", current.nr);
}

// ****** save exception
void oo_Heat::save_exceptions(void) {
	char ctmp[256];
	// --- open exception data file for writing
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/except.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "w");
	if (sd.data_file != NULL) {
		// -- walk through channels
		for (uint8_t i=0;i<rt.max_chn;i++) {
			// - walk through exceptions
			for (uint8_t e=0;e<rt.excount[i];e++) {
				sprintf(sd.data_line, "%x;%08x;\r\n", i, rt.exceptions[i][e]);
				sd.data_file_writeline();
			}
		}
		// -- close data file
		sd.data_file_close();
	}
}

// ****** load exception
void oo_Heat::load_exceptions(void) {
	char ctmp[256];
	// --- open exception data file for writing
	sprintf(ctmp, "/RTsd/data/%s/session%d/run%d/except.csv", event.name, session.nr, current.nr);
	sd.data_file_open(ctmp, "r");
	rt.excount[0] = 0;
	rt.excount[1] = 0;
	rt.excount[2] = 0;
	rt.excount[3] = 0;
	if (sd.data_file != NULL) {
		// -- walk through lines in file
		while(sd.data_file_getline_csv()) {
			rt.exceptions[sd.csv_array[0]][rt.excount[sd.csv_array[0]]] = sd.csv_array[1];
			printf("chn '%x' cnt '%x' exception '%08x'\r\n", sd.csv_array[0], rt.excount[sd.csv_array[0]], rt.exceptions[sd.csv_array[0]][rt.excount[sd.csv_array[0]]]);
			rt.excount[sd.csv_array[0]]++;
		}
		// -- close data file
		sd.data_file_close();
	}
	// --- write excceptions to rt
	rt.pd_set_exceptions();
	// --- increment exception mod counter
	rt.ex_mod_cnt++;
}

// ****** calc laps from hits
void oo_Heat::calc_laps(void) {
	// --- walk through channels
	for (uint8_t i=0;i<rt.max_chn;i++) {
		current.lapcount[i] = 0;
		// -- walk through hits
		for (uint8_t k=1;k<rt.hitcount[i];k++) {
			laps[i][k-1] = rt.hits[i][k] - rt.hits[i][k-1];
			current.lapcount[i]++;
		}
	}
}

// ****** calc position from laps and hits
void oo_Heat::calc_position(void) {
	uint8_t utmp = 0;
	// --- some starting point
	for (uint8_t i=0;i<rt.max_chn;i++) {
		current.pos_nr[i] = i;
		// -- determine time in ranking mode
		switch(session.mode) {
			// - training
			case SESSION_MODE_TRAIN:
				// calc heat time quali mode (from first pass)
				current.heat_time[i] = rt.hits[i][current.lapcount[i]] - rt.hits[i][0];
				break;
			// - quali
			case SESSION_MODE_QUALI:
				// calc heat time quali mode (from first pass)
				switch(event.quali_mode) {
					// use overtime for heat time
					case QUALI_OVERTIME:
						current.heat_time[i] = rt.hits[i][current.lapcount[i]] - rt.hits[i][0];
						break;
					// fastest consecutive laps
					case QUALI_CONSEC_LAPS:
						if (current.lapcount[i] <= event.quali_laps) {
							current.heat_time[i] = rt.hits[i][current.lapcount[i]] - rt.hits[i][0];
						} else {
							// find fastest consecutive n laps
							current.heat_time[i] = 0xffffffff;
							for(uint8_t l=0;l<(current.lapcount[i]-event.quali_laps);l++) {
								if (current.heat_time[i] > (rt.hits[i][l+event.quali_laps] - rt.hits[i][l])) {
									current.heat_time[i] = rt.hits[i][l+event.quali_laps] - rt.hits[i][l];
								}
							}
						}
						break;
				}
				break;
			// -race
			case SESSION_MODE_RACE:
				// calc heat time race mode (from start)
				current.heat_time[i] = rt.hits[i][current.lapcount[i]];
				break;
		}
	}
	// --- walk through channels
	for (uint8_t i=0;i<rt.max_chn;i++) {
		for (uint8_t k=0;k<rt.max_chn-1;k++) {
			// - compare purely by lap count
			if (current.lapcount[current.pos_nr[k]] < current.lapcount[current.pos_nr[k+1]]) {
				utmp = current.pos_nr[k];
				current.pos_nr[k] = current.pos_nr[k+1];
				current.pos_nr[k+1] = utmp;
			// - compare by time if laps are same
			} else if ((current.lapcount[current.pos_nr[k]] == current.lapcount[current.pos_nr[k+1]])
					&& (current.heat_time[current.pos_nr[k]] > current.heat_time[current.pos_nr[k+1]])) {
				utmp = current.pos_nr[k];
				current.pos_nr[k] = current.pos_nr[k+1];
				current.pos_nr[k+1] = utmp;
			}
		}
	}
}

// ****** calc fastest laps from laps
void oo_Heat::calc_fastest_laps(void) {
	uint8_t utmp = 0;
	// --- walk through channels
	for (uint8_t i=0;i<rt.max_chn;i++) {
		// -- set to somewhat sane defaults
		current.fastest_laps_time[i] = 0xffffffff;
		current.fastest_laps_lapnr[i] = 0xff;
		// -- walk through hits
		for (uint8_t k=0;k<current.lapcount[i];k++) {
			if (current.fastest_laps_time[i] > laps[i][k]) {
				current.fastest_laps_time[i] = laps[i][k];
				current.fastest_laps_lapnr[i] = k;
			}
		}
	}
	// --- determine position in running heat by fastest laps
	for (uint8_t i=0;i<rt.max_chn;i++) current.pos_fastest_lap[i] = i;
	for (uint8_t i=0;i<rt.max_chn;i++) {
		for (uint8_t k=0;k<rt.max_chn-1;k++) {
			if (current.fastest_laps_time[current.pos_fastest_lap[k]] > current.fastest_laps_time[current.pos_fastest_lap[k+1]]) {
				utmp = current.pos_fastest_lap[k];
				current.pos_fastest_lap[k] = current.pos_fastest_lap[k+1];
				current.pos_fastest_lap[k+1] = utmp;
			}
		}
	}
}
