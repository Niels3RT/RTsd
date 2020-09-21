#include "main.h"

// ****** init config
void oo_CFG::init(void) {
	char cfgname[] = "                ";
	// --- read wifi config file
	sprintf(cfgname, "/cfg_wifi.txt");
	sd.cfg_file_open(cfgname, "r");
	if (sd.cfg_file != NULL) {
		do {
			sd.cfg_file_getparm();
			// -- don't work on comments or eof
			if ((sd.cfg_parm[0] != '#') && (sd.cfg_parm[0] != 0xff)){
				//printf("%s : %s\r\n", sd.cfg_parm, sd.cfg_value);
				// -- wifi mode
				if (strcmp(sd.cfg_parm, CFG_WIFI_MODE) == 0) {
					// -- ap?
					if (strcmp(sd.cfg_value, CFG_WIFI_MODE_AP) == 0) wifi.ap_mode = 0;
					// -- sta?
					if (strcmp(sd.cfg_value, CFG_WIFI_MODE_STA) == 0) wifi.ap_mode = 1;
				}
				// -- wifi ap ssid
				if (strcmp(sd.cfg_parm, CFG_WIFI_AP_SSID) == 0) {
					strncpy(wifi.ap_ssid, sd.cfg_value, 40);
					printf("wifi ap ssid '%s'\r\n", wifi.ap_ssid);
				}
				// -- wifi ap key
				if (strcmp(sd.cfg_parm, CFG_WIFI_AP_KEY) == 0) {
					strncpy(wifi.ap_key, sd.cfg_value, 40);
					printf("wifi ap key '%s'\r\n", wifi.ap_key);
				}
				// -- wifi station ssid
				if (strcmp(sd.cfg_parm, CFG_WIFI_STA_SSID) == 0) {
					strncpy(wifi.sta_ssid, sd.cfg_value, 40);
					printf("wifi sta ssid '%s'\r\n", wifi.sta_ssid);
				}
				// -- wifi station key
				if (strcmp(sd.cfg_parm, CFG_WIFI_STA_KEY) == 0) {
					strncpy(wifi.sta_key, sd.cfg_value, 40);
					printf("wifi sta key '%s'\r\n", wifi.sta_key);
				}
			}
		} while(sd.cfg_parm[0] != 0xff);
		sd.cfg_file_close();
	} else {
		ESP_LOGE(TAG, "Failed to open wifi config file :(");
	}
	
	// --- rx frequencies
	//rt.chn_freq[0] = 5732;
	//rt.chn_freq[1] = 5769;
	//rt.chn_freq[2] = 5806;
	//rt.chn_freq[3] = 5843;
	rt.chn_freq[0] = 5658;
	rt.chn_freq[1] = 5695;
	rt.chn_freq[2] = 5732;
	rt.chn_freq[3] = 5769;
	rt.chn_freq[4] = 5806;
	rt.chn_freq[5] = 5843;
	rt.chn_freq[6] = 5880;
	rt.chn_freq[7] = 5917;
	
	// --- deadtime
	rt.deadtime = 8;
	
	// --- detect auto level percentage
	rt.det_quot_perc = 74;
	
	// --- read rt config file
	sprintf(cfgname, "/cfg_rt.txt");
	sd.cfg_file_open(cfgname, "r");
	if (sd.cfg_file != NULL) {
		do {
			sd.cfg_file_getparm();
			// -- don't work on comments or eof
			if ((sd.cfg_parm[0] != '#') && (sd.cfg_parm[0] != 0xff)){
				//printf("%s : %s\r\n", sd.cfg_parm, sd.cfg_value);
				// -- rx0 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX0) == 0) {
					rt.chn_freq[0] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX0, rt.chn_freq[0]);
				}
				// -- rx1 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX1) == 0) {
					rt.chn_freq[1] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX1, rt.chn_freq[1]);
				}
				// -- rx2 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX2) == 0) {
					rt.chn_freq[2] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX2, rt.chn_freq[2]);
				}
				// -- rx3 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX3) == 0) {
					rt.chn_freq[3] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX3, rt.chn_freq[3]);
				}
				// -- rx4 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX4) == 0) {
					rt.chn_freq[4] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX4, rt.chn_freq[4]);
				}
				// -- rx5 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX5) == 0) {
					rt.chn_freq[5] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX5, rt.chn_freq[5]);
				}
				// -- rx6 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX6) == 0) {
					rt.chn_freq[6] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX6, rt.chn_freq[6]);
				}
				// -- rx7 frequency
				if (strcmp(sd.cfg_parm, CFG_FREQ_RX7) == 0) {
					rt.chn_freq[7] = buf.buf2uint16_t_dec((uint8_t*)sd.cfg_value);
					printf("%s : %d\r\n", CFG_FREQ_RX7, rt.chn_freq[7]);
				}
			}
		} while(sd.cfg_parm[0] != 0xff);
		sd.cfg_file_close();
	} else {
		ESP_LOGE(TAG, "Failed to open rt config file :(");
	}
}

// ****** read calibration data
void oo_CFG::nvs_get_cal() {
	nvs_handle_t h_nvs;
	// --- use some sane defaults if something goes wrong
	for (uint8_t i=0;i<rt.max_chn;i++) {
		rt.rssi_base[i] = 0;
		rt.rssi_quot[i] = 0.5f;
	}
	
	// --- open nvs handle
	char ctmp[32];
	int32_t itmp = 0;
    if (nvs_open("storage", NVS_READONLY, &h_nvs) == ESP_OK) {
		// -- read
		for (uint8_t i=0;i<rt.max_chn;i++) {
			// - get base
			sprintf(ctmp, "rssi_base_%d", i);
			if (nvs_get_i32(h_nvs, ctmp, &itmp) == ESP_OK)
				rt.rssi_base[i] = (uint16_t)itmp;
			// - get quotient
			sprintf(ctmp, "rssi_quot_%d", i);
			if (nvs_get_i32(h_nvs, ctmp, &itmp) == ESP_OK)
				rt.rssi_quot[i] = *(float*)&itmp;
			// - debug output
			printf("%d;%05d;%f\r\n", i, rt.rssi_base[i], rt.rssi_quot[i]);
		}
	} else {
		ESP_LOGE(TAG,"Reading cal data from nvs failed!");
	}
	
	// --- close nvs handle
	nvs_close(h_nvs);
}

// ****** write calibration data
void oo_CFG::nvs_set_cal() {
	nvs_handle_t h_nvs;
	// --- open nvs handle
	char ctmp[32];
	if (nvs_open("storage", NVS_READWRITE, &h_nvs) == ESP_OK) {
		// -- write config data
		for (uint8_t i=0;i<rt.max_chn;i++) {
			// - set base
			sprintf(ctmp, "rssi_base_%d", i);
			nvs_set_i32(h_nvs, ctmp, rt.rssi_base[i]);
			// - set quotient
			sprintf(ctmp, "rssi_quot_%d", i);
			nvs_set_i32(h_nvs, ctmp, *(int32_t*)&rt.rssi_quot[i]);
		}
		nvs_commit(h_nvs);
	} else {
		ESP_LOGE(TAG,"Update cal data in nvs failed!");
	}

	// --- close nvs handle
	nvs_close(h_nvs);
}
