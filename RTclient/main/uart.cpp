#include "main.h"

// ****** init uart
void oo_UART::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init Info");

	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 122,		// prevent annoying warning
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_1, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
}

// ****** add crc to message in buffer
void oo_UART::add_crc(char *tbuf, uint8_t len) {
	// --- initial crc values
	uint8_t crcc = 0;
	
	// --- walk through bytes, calc crc
	for (uint8_t i=0;i<len;i++) {
		crcc = crcc ^ *(tbuf++);
	}
	
	// --- add crc to buffer
	sprintf(tbuf, "*%02x\r\n", crcc);
}

// ****** send message to rx
void oo_UART::send_msg(void) {
	char * tx_ptr = &utx_buf[0];
	char tx_ascii[256];

	//sprintf(tx_buf, "$V8X,23,sgshgdhdhdfhg*32");
	// --- print message to buffer
	//sprintf(tx_buf, "$V8X,00102020312048616C6C6F21736466202020202020202020202020202020202020202020202020202020");		// chn0
	//sprintf(tx_buf, "$V8X,0011 2020322048616C6C6F21736466202020202020202020202020202020202020202020202020202020");		// chn1
	
	for (uint8_t c=0;c<8;c++) {
		// --- set tx buffer pointer
		tx_ptr = &utx_buf[0];
	
		// --- clear arrays
		memset(utx_buf, 0x00, sizeof(utx_buf));
		memset(tx_ascii, 0x20, sizeof(tx_ascii));
		tx_ascii[48] = '\0';
		
		// --- select channel
		sprintf(tx_ptr, "$V8X,001%1x", c);
		tx_ptr += strlen(tx_ptr);
		//sprintf(tx_ptr, "2020322030313233343536373839303132333435363738393031323334353637383930313233343536");
		//sprintf(tx_ascii, "  3 Umit      P1 L4 1:23:456                      ");
		//if (info.heat.pilots_nr[c] < 0x20) {
			sprintf(tx_ascii, "  %1d %s                     ", c+1, info.pilots_data[info.heat.pilots_nr[c]].name);
		//}
		//printf("%s\r\n", tx_ascii);
		// --- encode string for msg
		uint8_t btmp = 0;
		for (uint8_t i=0;i<strlen(tx_ascii);i++) {
			//btmp = tx_ascii[i];
			//*(tx_ptr++) = (btmp >> 4) & 0x0f;
			//*(tx_ptr++) = btmp & 0x0f;
			btmp = (tx_ascii[i] >> 4) & 0x0f;
			if (btmp < 0x0a) {
				*(tx_ptr++) = btmp + 0x30;
			} else {
				*(tx_ptr++) = btmp + 0x57;
			}
			btmp = tx_ascii[i] & 0x0f;
			if (btmp < 0x0a) {
				*(tx_ptr++) = btmp + 0x30;
			} else {
				*(tx_ptr++) = btmp + 0x57;
			}
		}
		
		// --- add crc
		add_crc(&utx_buf[1], strlen(utx_buf)-1);
		
		// --- debug: print message to serial log
		//printf("%s", utx_buf);

		// Write data back to the UART
		uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	}
	
	// --- sky/noise all channels
	//sprintf(utx_buf, "$V8X,0002FF*18\r\n");		// sky for all
	sprintf(utx_buf, "$V8X,000200*18\r\n");		// noise for all
	// Write data back to the UART
	uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	
	// --- tune channels
	//sprintf(utx_buf, "$V8X,00060000000000000000*1C\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000000000000003*1F\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000000000000303*1C\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000000000030303*1F\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000000003030303*1C\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000000303030303*1F\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060000030303030303*1C\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060003030303030303*1F\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//vTaskDelay(30 / portTICK_PERIOD_MS);
	//sprintf(utx_buf, "$V8X,00060303030303030303*1C\r\n");
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
	//printf("%s", utx_buf);
	// Write data back to the UART
	//uart_write_bytes(UART_NUM_1, (const char *) utx_buf, strlen(utx_buf));
}
