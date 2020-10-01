#include "main.h"

// ****** init config
void oo_CFG::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init config");
	
	// --- copy default wifi ssid, pass
	strcpy(wifi_sta_ssid, DEFAULT_WIFI_STA_SSID);
	strcpy(wifi_sta_key, DEFAULT_WIFI_STA_PASS);
	
	// --- read wifi config from sd card
	read_wifi_cfg();
	
	// --- some vars to default
	max_chn = 4;
	
	// --- get config from tracker
	httpc.request_config();
}

// ****** read wifi config from sd card
void oo_CFG::read_wifi_cfg(void) {
	char cfgname[] = "                            ";
	
	// --- build filename and open file
	sprintf(cfgname, "/RTclient/cfg_wifi.txt");
	sd.cfg_file_open(cfgname, "r");
	if (sd.cfg_file != NULL) {
		do {
			sd.cfg_file_getparm();
			// -- don't work on comments or eof
			if ((sd.cfg_parm[0] != '#') && (sd.cfg_parm[0] != 0xff)){
				// -- wifi station ssid
				if (strcmp(sd.cfg_parm, CFG_WIFI_STA_SSID) == 0) {
					strncpy(wifi_sta_ssid, sd.cfg_value, 40);
					printf("wifi sta ssid '%s'\r\n", wifi_sta_ssid);
				}
				// -- wifi station key
				if (strcmp(sd.cfg_parm, CFG_WIFI_STA_KEY) == 0) {
					strncpy(wifi_sta_key, sd.cfg_value, 40);
					printf("wifi sta key '%s'\r\n", wifi_sta_key);
				}
			}
		} while(sd.cfg_parm[0] != 0xff);
		sd.cfg_file_close();
	} else {
		ESP_LOGE(TAG, "Failed to open wifi config file :(");
	}
}
