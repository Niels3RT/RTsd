#include "main.h"

// --- define some stuff to keep order
void rt_spi_pre_transfer_callback(spi_transaction_t *t);
void rt_spi_post_transfer_callback(spi_transaction_t *t);

spi_device_handle_t handle_spi;

// ****** init rt spi
void oo_RT_spi::init(void) {
	// --- some vars to default
	spi_tx_cmd = 0;
	spi_rx_data_max = 0;
	for (uint8_t i=0;i<16;i++) {
		spi_rx_data[i] = 0;
	}

	// --- write to log
	ESP_LOGI(TAG, "init rt spi");
	
	// --- gpio conf for dc pin
	gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;		//disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;				//set as output mode
    io_conf.pin_bit_mask = 1ULL<<PIN_NUM_DC;		//bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pull_down_en = (gpio_pulldown_t)0;		//disable pull-down mode
    io_conf.pull_up_en = (gpio_pullup_t)0;			//disable pull-up mode
    gpio_config(&io_conf);							//configure GPIO with the given settings

	esp_err_t ret;
    spi_bus_config_t buscfg = { };
        buscfg.miso_io_num=PIN_NUM_MISO;
        buscfg.mosi_io_num=PIN_NUM_MOSI;
        buscfg.sclk_io_num=PIN_NUM_CLK;
        buscfg.quadwp_io_num=-1;
        buscfg.quadhd_io_num=-1;
        buscfg.max_transfer_sz=32;
		
    spi_device_interface_config_t devcfg = { };
        devcfg.clock_speed_hz=10*1000*1000,					//Clock out at 10 MHz
		//devcfg.clock_speed_hz=6*1000*1000,					//Clock out at 6 MHz
		//devcfg.clock_speed_hz=2*1000*1000,					//Clock out at 2 MHz
		//devcfg.clock_speed_hz=1*1000*1000,				//Clock out at 1 MHz
        devcfg.mode=0,									//SPI mode 0
        devcfg.spics_io_num=PIN_NUM_CS,					//CS pin
        devcfg.queue_size=32,							//We want to be able to queue 7 transactions at a time
        devcfg.pre_cb=rt_spi_pre_transfer_callback,		//Specify pre-transfer callback to handle D/C line
		devcfg.post_cb=rt_spi_post_transfer_callback,	//Specify post-transfer callback to handle rx data

    // --- initialize the spi bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    // --- attach the device to the spi bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &handle_spi);
    ESP_ERROR_CHECK(ret);
}

// ****** read/write 8bit from address
void oo_RT_spi::transmit8(uint8_t adr, uint8_t value) {
	esp_err_t ret;
	static spi_transaction_t trans[2];
	memset(&trans[0], 0, sizeof(spi_transaction_t));
	
	trans[0].length = 16;
	trans[0].user = (void*)1;
	trans[0].tx_data[0] = adr;
	trans[0].tx_data[1] = value;
	trans[0].flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	
	// --- queue transmission
	ret=spi_device_queue_trans(handle_spi, &trans[0], portMAX_DELAY);
	assert(ret==ESP_OK);
	
	// --- wait transaction to finish
	spi_transaction_t *rtrans;
	ret=spi_device_get_trans_result(handle_spi, &rtrans, portMAX_DELAY);
}

// ****** read/write 16bit from address
uint16_t oo_RT_spi::transmit16(uint8_t adr, uint16_t value) {
	esp_err_t ret;
	static spi_transaction_t trans;
	memset(&trans, 0, sizeof(spi_transaction_t));
	
	trans.length = 24;
	trans.user = (void*)1;
	trans.tx_data[0] = adr;
	trans.tx_data[1] = (value>>8)&0xff;
	trans.tx_data[2] = value&0xff;
	trans.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	
	// --- queue transmission
	ret=spi_device_queue_trans(handle_spi, &trans, portMAX_DELAY);
	assert(ret==ESP_OK);
	
	// --- wait transaction to finish
	spi_transaction_t *rtrans;
	ret=spi_device_get_trans_result(handle_spi, &rtrans, portMAX_DELAY);
	
	// --- return shift out
	return((trans.rx_data[1]<<8) + trans.rx_data[2]);
}

// ****** read/write 24bit from address
uint32_t oo_RT_spi::transmit24(uint8_t adr, uint32_t value, uint8_t cd) {
	esp_err_t ret;
	uint32_t uitmp = 0;
	static spi_transaction_t trans[2];
	memset(&trans[0], 0, sizeof(spi_transaction_t));
	
	trans[0].length = 32;
	trans[0].user = (void*)&cd;
	trans[0].tx_data[0] = adr;
	trans[0].tx_data[1] = (value>>16)&0xff;
	trans[0].tx_data[2] = (value>>8)&0xff;
	trans[0].tx_data[3] = value&0xff;
	trans[0].flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	
	// --- queue transmission
	ret=spi_device_queue_trans(handle_spi, &trans[0], portMAX_DELAY);
	assert(ret==ESP_OK);
	
	// --- wait transaction to finish
	spi_transaction_t *rtrans;
	ret=spi_device_get_trans_result(handle_spi, &rtrans, portMAX_DELAY);
	
	// --- fetch result and return
	uitmp = (trans[0].rx_data[1]<<16) + (trans[0].rx_data[2]<<8) + trans[0].rx_data[3];
	
	return(uitmp);
}

// ****** write 128bit to spi, into status register, dc = 0
void oo_RT_spi::write128(char* out128) {
	esp_err_t ret;
	static spi_transaction_t trans;
	memset(&trans, 0, sizeof(spi_transaction_t));
	
	// -- empty buffer to shift out
	char in128[16];
	
	// --- config transaction
	trans.length = 128;
	trans.user = (void*)0;
	trans.rx_buffer = &in128[0];
	trans.tx_buffer = out128;
	trans.flags = 0x00;
	
	// --- queue transmission
	ret=spi_device_queue_trans(handle_spi, &trans, portMAX_DELAY);
	assert(ret==ESP_OK);
	
	// --- wait transaction to finish
	spi_transaction_t *rtrans;
	ret=spi_device_get_trans_result(handle_spi, &rtrans, portMAX_DELAY);
}

// ****** write 256bit to spi, into transfer register, dc = 0
void oo_RT_spi::write216(char* out216) {
	esp_err_t ret;
	static spi_transaction_t trans;
	memset(&trans, 0, sizeof(spi_transaction_t));
	
	// -- empty buffer to shift out
	char in216[27];
	
	// --- config transaction
	trans.length = 216;
	trans.user = (void*)0;
	trans.rx_buffer = &in216[0];
	trans.tx_buffer = out216;
	trans.flags = 0x00;
	
	// --- queue transmission
	ret=spi_device_queue_trans(handle_spi, &trans, portMAX_DELAY);
	assert(ret==ESP_OK);
	
	// --- wait transaction to finish
	spi_transaction_t *rtrans;
	ret=spi_device_get_trans_result(handle_spi, &rtrans, portMAX_DELAY);
}

// ****** This function is called (in irq context!) just before a transmission starts. It will
//        set the D/C line to the value indicated in the user field.
void rt_spi_pre_transfer_callback(spi_transaction_t *t) {
    int dc=(int)t->user;
    gpio_set_level((gpio_num_t)PIN_NUM_DC, dc);
}

// ****** This function is called (in irq context!) just after a transmission finishes.
void rt_spi_post_transfer_callback(spi_transaction_t *t) {
	uint8_t *tb = (uint8_t*)t->user;
	if (tb != 0) {
		rtspi.spi_rx_data_max++;
		rtspi.spi_rx_data_max &= 0x0f;
		rtspi.spi_rx_data[rtspi.spi_rx_data_max] = ((uint8_t)t->rx_data[1] << 8) + (uint8_t)t->rx_data[2];
		rtspi.spi_tx_cmd++;
		rtspi.spi_tx_cmd &= 0x0f;
	}
}
