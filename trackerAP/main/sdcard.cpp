#include "main.h"

// ****** init SD-card
void oo_SD::init(void) {
    esp_err_t ret = ESP_OK;
	
	// --- write to log
	ESP_LOGI(TAG, "init sd card");
	
    // --- init mount config/point
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    const char mount_point[] = "/sdcard";
    
	// --- init card & sdmmc host
	sdmmc_card_t* card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // --- init sdmmc slot config
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	slot_config.flags = SDMMC_HOST_SLOT_1 | SDMMC_SLOT_FLAG_INTERNAL_PULLUP | SDMMC_SLOT_NO_CD | SDMMC_SLOT_NO_WP;

	// --- mount sd card
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

	// --- card ok?
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize sd card (%s).", esp_err_to_name(ret));
        }
        return;
    }

    // --- print card properties
    sdmmc_card_print_info(stdout, card);
}

// ****** open config file
void oo_SD::cfg_file_open(char *tbuf, char const * rw) {
	// --- build filename
	char path[256];
	//sprintf(path, "/sdcard/RTsd/cfg_%s.txt", tbuf);
	sprintf(path, "/sdcard/RTsd%s", tbuf);

	// --- dump wifi config file
	cfg_file = fopen(path, rw);

	// --- ok?
	if (cfg_file == NULL) {
		ESP_LOGE(TAG, "Failed to open config file '%s'", path);
	}
}

// ****** open data file
void oo_SD::data_file_open(const char *tbuf, char const * rw) {
	// --- build filename
	char path[256];
	sprintf(path, "/sdcard/RTsd/data/%s", tbuf);

	// --- dump wifi config file
	data_file = fopen(path, rw);
	
	// --- ok?
	if (data_file == NULL) {
		ESP_LOGE(TAG, "Failed to open data file '%s'", path);
	}
}

// ****** get next parameter from config file
void oo_SD::cfg_file_getparm() {
	char line[256];
	uint8_t len = 0;
	
	if (fgets(line, sizeof(line), cfg_file) != NULL) {
		// -- is comment line?
		bool is_comment = false;
		char * ptmp = strchr(line, '=');
		char * ctmp = strchr(line, '#');
		if (ctmp != NULL) {
			if (ptmp == NULL) {
				is_comment = true;
			} else {
				if (ptmp > ctmp) is_comment = true;
			}
		}
		if (!is_comment) {
			// -- parameter identifier
			len = ptmp - &line[0];
			if (len > 39) len = 39;
			strncpy(cfg_parm, line, len);
			cfg_parm[len] = '\0';
			ptmp++;
			// -- parameter value
			len = strchr(ptmp, ';') - ptmp;
			if (len > 40) len = 40;
			//cfg_value[0] = '\0';
			strncpy(cfg_value, ptmp, len);
			cfg_value[len] = '\0';
		} else {
			cfg_parm[0] = '#';
		}
	} else {
		cfg_parm[0] = 0xff;
	}
}

// ****** get next line from data file
bool oo_SD::data_file_getline() {
	// --- read line of data file
	if (fgets(data_line, sizeof(data_line), data_file) != NULL) {
		// -- is comment line?
		char * ctmp = strchr(data_line, '#');
		if (ctmp != NULL) {
			data_line[0] = '\0';
		}
		return(true);
	} else {
		data_line[0] = '\0';
		return(false);
	}
}

// ****** get next line from data file and parse csv into array !HEX!
bool oo_SD::data_file_getline_csv() {
	char * ptmp = &data_line[0];
	char * ptop = &data_line[255];
	uint8_t cnt = 0;
	uint32_t etmp = 0;
	// --- clear transfer array
	for (uint8_t i=0;i<8;i++) csv_array[i] = 0;
	// --- read line of data file
	if(data_file_getline()) {
		// -- parse csv line
		bool go = true;
		bool gof = true;
		ptop = ptmp + strlen(data_line);
		//printf("'%s'\r\n", data_line);
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
			//printf("_%c_", *ptmp);
			// - advance pointer?
			if ((*ptmp == 0x3b) && (cnt < 7) && (ptmp < ptop)) {
				cnt++;
				ptmp++;
			} else {
				go = false;
			}
		} while(go);
		//printf("\r\n");
		//printf("csv line count '%d'\r\n", cnt);
		return(true);
	} else {
		//printf("\r\n");
		return(false);
	}
}

// ****** write line of data file
void oo_SD::data_file_writeline(void) {
	// --- print for debugging
	//printf("%s", data_line);
	// --- write to file
	fputs(data_line, data_file);
	
	return;
}

// ****** write line of config file
void oo_SD::cfg_file_writeline(void) {
	// --- print for debugging
	printf("%s", data_line);
	// --- write to file
	fputs(data_line, cfg_file);
	
	return;
}

// ****** close config file
void oo_SD::cfg_file_close() {
	fclose(cfg_file);
}

// ****** close data file
void oo_SD::data_file_close() {
	fclose(data_file);
}

// ****** open directory
bool oo_SD::dir_open(const char *tbuf) {
	// --- build filename
	char path[256];
	sprintf(path, "/sdcard%s", tbuf);
	
	// --- try to open directoy
	folder = opendir(path);
	
	// --- did it work?
	if(folder == NULL) {
		printf("Unable to read directory '/sdcard%s'\r\n", tbuf);
		return(false);
	} else {
		//printf("Directory '/sdcard%s' is opened!\r\n", tbuf);
		return(true);
	}
}

// ****** get directory entry
void oo_SD::dir_get_entry(void) {
	// --- read direcotry entry
	entry = readdir(folder);
}

// ****** open directory
void oo_SD::dir_close(void) {
	// --- close dir
	closedir(folder);
}

// ****** create directory
void oo_SD::dir_create(char *tbuf) {
	char ctmp[256];
	// --- build path
	sprintf(ctmp, "/sdcard%s", tbuf);
	// --- create dir
	mkdir(ctmp, 0777);
}
