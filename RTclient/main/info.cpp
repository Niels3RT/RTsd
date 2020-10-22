#include "main.h"

// ****** init info
void oo_Info::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init Info");

	// --- some vars to default
	pilot_count = 0;
	use_event_mode = 0;
	event_mod_cnt = 0xff;
	event_mod_cnt_new = 0xff;
	session_mod_cnt = 0xff;
	session_mod_cnt_new = 0xff;
	heat_mod_cnt = 0xff;
	heat_mod_cnt_new = 0xff;
}

// ****** calc laps from hits
void oo_Info::calc_laps(void) {
	// --- clear some stuff
	memset(lapcount, 0, sizeof(lapcount));
	memset(laps, 0, sizeof(laps));

	// --- walk through channels
	for (uint8_t i=0;i<cfg.max_chn;i++) {
		lapcount[i] = 0;
		// -- walk through hits
		for (uint8_t k=1;k<hitcount[i];k++) {
			laps[i][k-1] = hits[i][k] - hits[i][k-1];
			lapcount[i]++;
		}
	}
}

// ****** print heat info
void oo_Info::print_heat(void) {
	for (uint8_t i=0;i<cfg.max_chn;i++) {
		// --- pilot names
		printf("chn %d - %s\r\n", i, pilots_data[heat.pilots_nr[i]].name);
		
		// --- laps
		for (uint8_t l=0;l<lapcount[i];l++) {
			printf("   %d %d\r\n", l+1, laps[i][l]);
		}
	}
	
	// --- fill scrolltext with pilot names
	char ctmp[512];
	sprintf(ctmp, "1.%s  2.%s  3.%s  4.%s  5.%s  6.%s  7.%s  8.%s  ", pilots_data[heat.pilots_nr[0]].name,
																		pilots_data[heat.pilots_nr[1]].name,
																		pilots_data[heat.pilots_nr[2]].name,
																		pilots_data[heat.pilots_nr[3]].name,
																		pilots_data[heat.pilots_nr[4]].name,
																		pilots_data[heat.pilots_nr[5]].name,
																		pilots_data[heat.pilots_nr[6]].name,
																		pilots_data[heat.pilots_nr[7]].name);
	ws.write_scrolltext(&ctmp[0], strlen(ctmp));
}
