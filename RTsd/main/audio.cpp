#include "main.h"

// ****** init audio sample
void oo_Audio::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init audio");
	
	// --- only init audio if not running on Spartan6
	if (rt.type == 0) {
		ESP_LOGE(TAG, "I2S Audio not supported on Spartan6 :(");
		return;
	}
	
	// --- some vars
	//sample_adr = 0x8fffff;
	sample_adr = 0x800000;
	//sample_cnt = 0;
	
	// --- load samples?
	if (use_i2s) {
		// -- time sd card read
		int64_t timer_start = esp_timer_get_time();
		
		// -- load sample files
		sample_file_load(WAVE_GO, 0);
		sample_file_load(WAVE_1, 1);
		sample_file_load(WAVE_2, 2);
		sample_file_load(WAVE_3, 3);
		sample_file_load(WAVE_STOP, 4);
		sample_file_load(WAVE_GUN, 5);
		sample_file_load(WAVE_DING, 6);
		sample_file_load(WAVE_DONG, 7);
		sample_file_load(WAVE_N1, 8);
		sample_file_load(WAVE_N2, 9);
		sample_file_load(WAVE_N3, 10);
		sample_file_load(WAVE_N4, 11);
		sample_file_load(WAVE_N5, 12);
		sample_file_load(WAVE_N6, 13);
		sample_file_load(WAVE_N7, 14);
		sample_file_load(WAVE_N8, 15);
		
		// -- print sample load time
		int64_t timer_delta = esp_timer_get_time() - timer_start;
		printf("I2S: card read samples time %llums\r\n", timer_delta / 1000);
	} else {
		printf("I2S: skip reading samples\r\n");
	}
}

// ****** play sample
void oo_Audio::play_sample(uint8_t nr) {
	// --- only play audio if not running on Spartan6
	if (rt.type == 0) {
		ESP_LOGI(TAG, "I2S Audio not supported on Spartan6 :(");
		return;
	}

	rtspi.transmit8(RT_PLAY_SAMPLE, nr);
	printf("Audio: play sample nr %d\r\n", nr);
}

// ****** copy wave file data to sample memory
void oo_Audio::sample_file_load(const char *sample_name, uint8_t sample_nr) {
	char ctmp[256];
	int len = 0;
	bool wave_is_good = true;
	uint16_t sample_buf[512];
	char spi_buf[16];
	
	// --- remember start address
	uint32_t sample_adr_start = sample_adr;
	
	// --- build sample name path
	sprintf(ctmp, "/RTsd/audio/%s", sample_name);
	
	// --- open sample file
	sd.data_file_open(ctmp, "rb");
	if (sd.data_file != NULL) {
		printf("sample file '%s' opened!\r\n", ctmp);
		// -- time sd card read
		//int64_t timer_start = esp_timer_get_time();
		// -- read and check headers
		for (uint8_t i=0;i<16;i++) {
			// - read 4 byte chunks
			len = fread(ctmp, sizeof *ctmp, 4, sd.data_file);
			// - check if 4 bytes read
			if (len == 4) {
				//printf("wave file i=%d - %d : %02x %02x %02x %02x\r\n", i, len, ctmp[0], ctmp[1], ctmp[2], ctmp[3]);
				// - check RIFF
				if (i == 0) {
					if ((ctmp[0]==0x52) && (ctmp[1]==0x49) && (ctmp[2]==0x46) && (ctmp[3]==0x46)) {
						//printf("is RIFF!\r\n");
					} else {
						// read strange things, break
						wave_is_good = false;
						break;
					}
				}
				// - check WAVE
				if (i == 2) {
					if ((ctmp[0]==0x57) && (ctmp[1]==0x41) && (ctmp[2]==0x56) && (ctmp[3]==0x45)) {
						//printf("is WAVE!\r\n");
					} else {
						// read strange things, break
						wave_is_good = false;
						break;
					}
				}
				// - try to find sample data
				if ((ctmp[0]==0x64) && (ctmp[1]==0x61) && (ctmp[2]==0x74) && (ctmp[3]==0x61)) {
					//printf("Sample data found at %d!\r\n", i);
					break;
				}
			} else {
				// read strange things, break
				wave_is_good = false;
				break;
			}
			// -- didnt find data, stop looking
			if (i == 15) {
				wave_is_good = false;
				break;
			}
		}
		// -- if wave is good, start reading samples
		if (wave_is_good) {
			//printf("WAVE file is good! Go on :)\r\n");
			// -- read sample data len
			uint32_t sample_len = 0;
			len = fread(&sample_len, sizeof sample_len, 1, sd.data_file);
			sample_len >>= 1;
			//printf("Sample data len %zu!\r\n", sample_len);
			// -- read sample data, copy to fpga ddr
			uint32_t sample_len_read = 0;
			do {
				memset(&sample_buf, 0, sizeof(sample_buf));
				//len = fread(sample_buf, sizeof *sample_buf, 512, sd.data_file);
				len = fread(sample_buf, 2, 512, sd.data_file);
				sample_len_read += len;
				// - write buffer to fpga
				uint16_t s_pos = 0;
				uint8_t b_pos = 0;
				for (uint8_t i=0;i<64;i++) {
				//for (uint8_t i=0;i<2;i++) {
					// write samples to buffer
					b_pos = 0;
					for (uint8_t k=0;k<8;k++) {
						//spi_buf[b_pos++] = sample_buf[s_pos++] & 0xff;
						//spi_buf[b_pos++] = sample_buf[s_pos] >> 8;
						// silence
						spi_buf[b_pos++] = sample_buf[s_pos] >> 8;
						spi_buf[b_pos++] = sample_buf[s_pos++] & 0xff;
						//if (b_pos > 2) printf("%04x: %02x %02x\r\n", sample_buf[s_pos-1], spi_buf[b_pos-2], spi_buf[b_pos-1]);
						// rng
						//spi_buf[b_pos++] = esp_random() & 0xff;
						//spi_buf[b_pos++] = esp_random() & 0xff;
					}
					rtspi.write128(&spi_buf[0]);									// write 128bit word to transfer register
					rtspi.transmit24(RT_REG2SDRAM_RSSI, sample_adr++, 0);			// set sdram address (rssi block nr)
				}
				//printf("sample data %d : %04x %04x %04x %04x\r\n", len, sample_buf[0], sample_buf[1], sample_buf[2], sample_buf[3]);
			} while(len > 0);
			//int64_t timer_delta = esp_timer_get_time() - timer_start;
			////printf("Sample data read len %zu, sample adr %zu!\r\n", sample_len_read, sample_adr);
			printf("Sample data read len %zu, sample adr %08x!\r\n", sample_len_read, sample_adr_start);
			//printf("card read time %llu\r\n", timer_delta);
			
			// -- update fpga sample table
			memset(&spi_buf, 0, sizeof(spi_buf));
			// - prepare sample len, 20 bits
			spi_buf[15] = sample_len_read & 0xff;
			spi_buf[14] = (sample_len_read >> 8) & 0xff;
			spi_buf[13] = (sample_len_read >> 16) & 0x0f;
			// - prepare sample adr, 24 bits
			spi_buf[13] |= (sample_adr_start << 4) & 0xf0;
			spi_buf[12] = (sample_adr_start >> 4) & 0xff;
			spi_buf[11] = (sample_adr_start >> 12) & 0xff;
			spi_buf[10] = (sample_adr_start >> 20) & 0x0f;
			// DEBUG
			//spi_buf[10] = 0xff;
			//spi_buf[11] = 0xff;
			//spi_buf[12] = 0x00;
			//spi_buf[13] = 0x00;
			//spi_buf[14] = 0xff;
			//spi_buf[15] = 0xff;
			// - write entry
			rtspi.write128(&spi_buf[0]);									// write 128bit word to transfer register
			rtspi.transmit16(RT_REG2SAMPLE_TABLE, sample_nr);				// set sample table bram address
			//sample_adr
		} else {
			printf("WAVE file ain't no good :(\r\n");
		}
		
		// -- close data file
		sd.data_file_close();
	}
}
