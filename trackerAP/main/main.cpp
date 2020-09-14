#include "main.h"

// --- declare stuff
// -- objects
oo_WiFi wifi;
oo_RT_spi rtspi;
oo_Timer timer;
oo_oLed oled;
oo_Buf buf;
oo_HTTP http;
oo_RT rt;
oo_SD sd;
oo_CFG cfg;
oo_Heat heat;
oo_Session session;
oo_Event event;

// -- handles
esp_event_loop_handle_t main_loop_handle;

ESP_EVENT_DEFINE_BASE(TRACKER_EVENTS);         // declaration of the tracker events family

// ****** main event handler
void main_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	// --- do heat commit?
	if (event_id == EVENT_RT_DO_COMMIT) {
		heat.commit();
	}
	
	// --- open heat?
	if (event_id == EVENT_RT_DO_OPEN) {
		heat.open();
	}

	// --- print task list
	//printf( "Task Name\tStatus\tPrio\tHWM\tTask\tAffinity\n");
	//char stats_buffer[1024];
	//vTaskList(stats_buffer);
	//printf("%s\n", stats_buffer);
	// --- read status from rt
	//oled.print_dec_16u(11, 0, timer.tick & 0xffff);
	//oled.print_hex_16u(12, 1, timer.tick & 0xffff);
	//oled.print_dec_8u(13, 2, timer.tick & 0xff);
	//oled.writefb();
	
	// -- read line (4 words a 16bits) of rssi from sdram
	//char trssi[8];
	//rtspi.transmit24(RT_SDRAM2REG, 7394, 0);		// set sdram address (rssi block nr)
	//rtspi.read64(&trssi[0]);						// fetch 64bit word from transfer register
	//uint8_t * utmp = (uint8_t*)&trssi[7];
	//printf("%02x\r\n", *utmp);
	
	//rt.tune_rx(0, 5732);
	//rt.tune_rx(1, 5769);
	//rt.tune_rx(2, 5806);
	//rt.tune_rx(3, 5843);
	
		// --- set normalise parmameters
	// -- base
	//rtspi.transmit24(RT_NORM_BASE+0, 11832, 1);
	//rtspi.transmit24(RT_NORM_BASE+1, 12503, 1);
	//rtspi.transmit24(RT_NORM_BASE+2, 32763, 1);
	//rtspi.transmit24(RT_NORM_BASE+3, 22661, 1);
	//// -- quotient
	//float ftmp = 4.00F;
	//rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);	// write float into transfer register
	//rtspi.transmit_cmd(RT_QUOT_BASE+0);			// copy transfer register to output
	//ftmp = 3.58F;
	//rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);	// write float into transfer register
	//rtspi.transmit_cmd(RT_QUOT_BASE+1);			// copy transfer register to output
	//ftmp = 0.01F;
	//rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);	// write float into transfer register
	//rtspi.transmit_cmd(RT_QUOT_BASE+2);			// copy transfer register to output
	//ftmp = 7.02F;
	//rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);	// write float into transfer register
	//rtspi.transmit_cmd(RT_QUOT_BASE+3);			// copy transfer register to output
	
	//float ftmp = 3.94F;
	//rtspi.transmit32(0, *(uint32_t*)&ftmp, 1);
	
	//rt.fetch_minmax();
	//for (uint8_t i=0;i<4;i++) {
	//	printf("min %05d  max %05d  delta %05d  quot %2f\r\n", rt.rssi_min[i], rt.rssi_max[i], rt.rssi_max[i] - rt.rssi_min[i], ((float)(rt.rssi_max[i] - rt.rssi_min[i]))/4096);
	//}
	
	//float ftmp = 3.94F;
	//printf("ftmp %08x", *(uint32_t*)&ftmp);
	
	//rt.get_state();
	// --- write to log
	//ESP_LOGI(TAG, "main event! :) %02x - %04x", uitmp, rtspi.spi_rx_data[rtspi.spi_rx_data_max]);
}

// ****** main
void app_main(void) {

    // --- Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	// --- init main event loop
	ESP_LOGI(TAG, "init event loop");
	esp_event_loop_args_t loop_args = {
		.queue_size = 32,
		.task_name = "main_loop_task",
		.task_priority = uxTaskPriorityGet(NULL),
		//.task_stack_size = 2048,
		.task_stack_size = 4096,
		.task_core_id = tskNO_AFFINITY
    };
	ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &main_loop_handle));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_TIMER_MAIN, main_event_handler, main_loop_handle, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_COMMIT, main_event_handler, main_loop_handle, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_RT_DO_OPEN, main_event_handler, main_loop_handle, NULL));
	
	// --- wait some for fpga to start up
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	
	// --- init buffer
	buf.init();
	
	// --- init sd card
	sd.init();
	
	// --- init config, read cfg files
	cfg.init();
	
	// --- init rt spi
	rtspi.init();
	
	// --- init rt
	rt.init();
	
	// --- init event
	event.init();
	
	// --- init session
	session.init();
	
	// --- init heat
	heat.init();
	
	// --- init timer
	timer.init();
	
	// --- init wifi
	wifi.init();
	
	// --- init http
	http.init();
	
	// --- init runtime stats
	//sync_stats_task = xSemaphoreCreateBinary();
	//xTaskCreatePinnedToCore(stats_task, "stats", 4096, NULL, STATS_TASK_PRIO, NULL, tskNO_AFFINITY);
 //   xSemaphoreGive(sync_stats_task);
}
