#include "main.h"

// ****** init racetimer
void oo_RT::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init rt");
	
	// --- some vars
	state = 0;
	count = 0;
	for (uint8_t i=0;i<4;i++) {
		trg_level[i] = 0;
		rssi_min[i] = 0;
		rssi_max[i] = 0;
		hitcount[i] = 0;
	}
	ex_mod_cnt = 0;
	
	// --- tune rxes
	tune_rx_all();
	
	// --- read tracker state
	//get_state();
	
	// --- clear tracker state
	clear(false);
	
	// --- clear minmax
	clear_minmax();
	do_calib = false;
	
	// --- read and set normalise parmameters
	cfg.nvs_get_cal();
	write_cal_data();
	
	// --- set trigger levels
	rt.pd_set_tlevel(0, 0x800);
	rt.pd_set_tlevel(1, 0x800);
	rt.pd_set_tlevel(2, 0x800);
	rt.pd_set_tlevel(3, 0x800);

	// --- set auto trigger level factor
	pd_set_auto_quotient();
	
	// --- set deadtime
	pd_set_deadtime();
	
	// --- det autolevel minimum
	det_auto_min = 20;
	pd_set_auto_min();
}

// ****** get racetimer state
void oo_RT::get_state(void) {
	// --- read state from spi
	uint32_t uitmp = rtspi.transmit24(RT_READ_STATE, 0, 1);
	state = uitmp & 0xf;
	count = uitmp >> 4;
	
	//printf("rt get state %01x count %08x\r\n", state, count);
}

// ****** set rt state via spi
void oo_RT::set_state(void) {
	rtspi.transmit8(RT_WRITE_STATE, state);
	//printf("rt set state %01x\r\n", state);
}

// ****** start run
void oo_RT::start(void) {
	// --- set state and write to spi
	state = 1;
	set_state();
}

// ****** clear run
void oo_RT::clear(bool keep_pilots) {
	// --- set state and write to spi
	state = 0;
	set_state();
	for (uint8_t i=0;i<4;i++) {
		trg_level[i] = det_auto_min;
		rssi_min[i]  = 0;
		rssi_max[i]  = 0;
		hitcount[i]  = 0;
	}
	//ex_mod_cnt = 0;
	// --- clear current heat
	uint8_t nrtmp = heat.current.nr;
	if (keep_pilots) {
		uint8_t p_save[4];
		p_save[0] = heat.current.pilots_nr[0];
		p_save[1] = heat.current.pilots_nr[1];
		p_save[2] = heat.current.pilots_nr[2];
		p_save[3] = heat.current.pilots_nr[3];
		heat.current = session.st_heat_empty;
		heat.current.pilots_nr[0] = p_save[0];
		heat.current.pilots_nr[1] = p_save[1];
		heat.current.pilots_nr[2] = p_save[2];
		heat.current.pilots_nr[3] = p_save[3];
	} else {
		heat.current = session.st_heat_empty;
		heat.current.pilots_nr[0] = 0;
		heat.current.pilots_nr[1] = 1;
		heat.current.pilots_nr[2] = 2;
		heat.current.pilots_nr[3] = 3;
	}
	heat.current.nr = nrtmp;
	// --- reset counter in tracker
	count = 0;
	set_count();
	// --- clear peak detect ram
	pd_clear();
}

// ****** stop run
void oo_RT::stop(void) {
	// --- set state and write to spi
	state = 2;
	set_state();
}

// ****** set rt counter through spi
void oo_RT::set_count(void) {
	// --- set state and write to spi
	rtspi.transmit24(RT_WRITE_COUNT, count, 1);
}

// ****** tune all rxes from freq array
void oo_RT::tune_rx_all() {
	tune_rx(0, chn_freq[0]);
	tune_rx(1, chn_freq[1]);
	tune_rx(2, chn_freq[2]);
	tune_rx(3, chn_freq[3]);
}

// ****** tune rxes via spi
void oo_RT::tune_rx(uint8_t target_rx, uint16_t freq) {
	switch(target_rx) {
		// -- rx 0
		case 0:
			rtspi.transmit16(RT_TUNE_RX0, freq);
			break;
		// -- rx 1
		case 1:
			rtspi.transmit16(RT_TUNE_RX1, freq);
			break;
		// -- rx 2
		case 2:
			rtspi.transmit16(RT_TUNE_RX2, freq);
			break;
		// -- rx 3
		case 3:
			rtspi.transmit16(RT_TUNE_RX3, freq);
			break;
	}
}

// ****** clear min max mimum rssi for channels
void oo_RT::clear_minmax(void) {
	// --- clear minmax
	rtspi.transmit_cmd(RT_CLR_MINMAX);
}

// ****** fetch min max mimum rssi for channels
void oo_RT::fetch_minmax(void) {
	// --- read min max per channel
	for (uint8_t i=0;i<4;i++) {
		rssi_min[i] = rtspi.transmit16(RT_MIN_BASE+i, 0);
		rssi_max[i] = rtspi.transmit16(RT_MAX_BASE+i, 0);
	}
}

// ****** write calibration data to tracker via spi
void oo_RT::write_cal_data(void) {
	// --- read min max per channel
	for (uint8_t i=0;i<4;i++) {
		rtspi.transmit24(RT_NORM_BASE+i, rssi_base[i], 1);
		rtspi.transmit32(0, *(uint32_t*)&rssi_quot[i], 1);	// write float into transfer register
		rtspi.transmit_cmd(RT_QUOT_BASE+i);					// copy transfer register to output
	}
}

// ****** set peak detect trigger level
void oo_RT::pd_set_tlevel(uint8_t chn, uint16_t level) {
	// --- remember in array
	trg_level[chn] = level;
	// --- determine channel
	uint8_t adr = 0;
	switch(chn) {
		case 0:
			adr = RT_TRG_LEVEL0;
			break;
		case 1:
			adr = RT_TRG_LEVEL1;
			break;
		case 2:
			adr = RT_TRG_LEVEL2;
			break;
		case 3:
			adr = RT_TRG_LEVEL3;
			break;
	}
	// --- set trigger level
	rtspi.transmit16(adr, level);
	
	//printf("tl %02x %04x\r\n", adr, trg_level[chn]);
}

// ****** get peak detect trigger level
void oo_RT::pd_get_tlevels() {
	// --- walk through channels
	for (uint8_t i=0;i<4;i++) {
		trg_level[i] = rtspi.transmit16(RT_TRIGGER_BASE+i, 0);
	}
}

// ****** set deadtime
void oo_RT::pd_set_deadtime(void) {
	// --- set dead time
	rtspi.transmit24(RT_PD_DEADTIME, deadtime*10, 1);
}

// ****** set auto min level
void oo_RT::pd_set_auto_min(void) {
	// --- calc and limit level from percentage
	uint16_t tmp = 41 * det_auto_min;
	if (tmp > 0xfff) tmp = 0xfff;
	// --- set auto min level
	rtspi.transmit16(RT_PD_AUTO_MIN, tmp);
}

// ****** set peak detect fixed trigger level mode
void oo_RT::pd_set_fixed_mode(uint8_t mode) {
	// --- set mode bit0 chn0,...   , 0 means auto, 1 is fixed trigger level mode
	rtspi.transmit8(RT_PD_DET_FIXED, mode);
	
	printf("tm %02x %01x\r\n", RT_PD_DET_FIXED, mode);
}

// ****** set quotient for peak detect trigger auto level
void oo_RT::pd_set_auto_quotient(void) {
	float ftmp = 1/((float)det_quot_perc / 100);
	rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);		// write float into transfer register
	rtspi.transmit_cmd(RT_PD_AUTO_FACTOR);			// copy transfer register to output
}

// ****** set quotient for peak detect trigger auto level
void oo_RT::pd_set_exceptions(void) {
	// --- clear exception bram counters
	rtspi.transmit_cmd(RT_PD_EX_CLEAR);						// clear max registers
	// --- walk through exceptions
	for (uint8_t i=0;i<4;i++) {
		// -- write valid exceptions
		for (uint8_t k=0;k<excount[i];k++) {
			rtspi.transmit32(0, exceptions[i][k], 1);		// write float into transfer register
			rtspi.transmit_cmd(RT_PD_EXC_BASE+i);			// copy transfer register to ex bram
		}
		// -- write limiter
		if (excount[i] < 32) {
			rtspi.transmit32(0, 0x00ffffff, 1);				// write float into transfer register
			rtspi.transmit_cmd(RT_PD_EXC_BASE+i);			// copy transfer register to ex bram
		}
	}
}

// ****** start peak detect
void oo_RT::pd_start(void) {
	ESP_LOGI(TAG,"start peak detect");
	// --- start peak detect
	rtspi.transmit_cmd(RT_PD_START);
}

// ****** clear peak detect ram
void oo_RT::pd_clear(void) {
	// --- clear peak detect
	rtspi.transmit_cmd(RT_PD_CLEAR);
	// --- set peak detect fixed level mode to auto for all channels
	pd_set_fixed_mode(0x00);
	// -- clear exceptions
	for (uint8_t i=0;i<4;i++) {
		excount[i] = 0;
		for (uint8_t k=0;k<32;k++) {
			exceptions[i][k] = 0x00ffffff;
		}
	}
}

// ****** peak detect is ready?!
bool oo_RT::pd_isready(void) {
	// --- is ready?
	uint32_t uitmp = rtspi.transmit32(RT_READ_STATE, 0, 1);
	if (uitmp & RT_STATUS_PDREADY) {
		return(true);
	} else {
		ESP_LOGE(TAG,"peak detect not ready :( %08x", uitmp);
		return(false);
	}
}

// ****** fetch laps from peak detect
void oo_RT::pd_fetch(void) {
	uint32_t hit = 0;
	uint16_t adr = 0;
	
	ESP_LOGI(TAG,"fetch peak detect");
	
	// --- limit time for laps in quali mode?
	uint32_t time_limit = 0xffffffff;
	if ((session.mode == SESSION_MODE_QUALI) && (event.quali_mode == QUALI_OVERTIME)) {
		time_limit = (uint32_t)event.current.quali_otime * 1000;
	}
	
	// --- walk through channels
	for (uint8_t k=0;k<4;k++) {
		hitcount[k] = 0;
		adr = k * 0x100;
		// -- walk through hits
		for (uint8_t i=0;i<128;i++) {
			// --- write bram address
			rtspi.transmit16(RT_PEAK2REG, adr+i);
			// --- read transfer register
			char thit[8];
			rtspi.readn(&thit[0], 24);
			hit = (thit[0]<<16) + (thit[1]<<8) + thit[2];
			if (hit != 0xffffff) {
				//printf("%d;%02x;%08d\r\n", k, i, hit);
				hitcount[k]++;
				hits[k][i] = hit;
				// - is hit within quali overtime?
				if ( (hits[k][i] - hits[k][0]) > time_limit) {
					//printf("quali overtime reached %d;%02x;%08d\r\nIgnore further hits.\r\n", k, i, hit);
					i = 250;
				}
			} else {
				i = 250;
			}
		}
	}
}
