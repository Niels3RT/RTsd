#include "main.h"

// --- declare stuff
// -- objects
oo_WiFi wifi;
oo_Timer timer;
oo_HTTPC httpc;
oo_DNSs dnss;
oo_CFG cfg;
oo_Info info;
oo_UART uart;
oo_RT_spi rtspi;
oo_SD sd;
oo_WS ws;
oo_Font font;

// -- handles
esp_event_loop_handle_t main_loop_handle;

ESP_EVENT_DEFINE_BASE(TRACKER_EVENTS);         // declaration of the tracker events family

// ****** main event handler
void main_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	
	// --- open heat?
	if (event_id == EVENT_TIMER_MAIN) {
		// -- check mod counters
		httpc.handle_mod_cnt();
		// -- calc delta time, in us
		int64_t time_last_good_delta = esp_timer_get_time() - httpc.time_last_good_result;
		// -- get results if everything else is ok and delta time from last result is big enough
		if ((httpc.tx_buf_work == httpc.tx_buf_top) && (time_last_good_delta > 2500000) && (httpc.in_progress == false)) {
			//printf("DeltaTest '%llu'\r\n", time_last_good_delta);
			// - request results
			httpc.request_results();
		}
		// DEBUG
		if ((timer.tick & 0x07) == 0) {
			if (ws.tmp_cnt < 100) {
				ws.tmp_cnt++;
			} else {
				ws.tmp_cnt = 0;
			}
			//st_rgb ctmp = { 8, 8, 8 };
			st_rgb ctmp = { 8, 0, 0 };	// red
			//st_rgb ctmp = { 0x0e, 0x00, 0x07 };	// pink
			ws.print_7s_2d(ctmp, ws.tmp_cnt);
			//ws.print_digit(0, ctmp, ws.tmp_cnt);
			//ws.print_digit(1, ctmp, ws.tmp_cnt);
			ws.transfer_fb(0, 64);
			ws.transfer_fb(1, 64);
			ws.trigger_write(0);
			ws.trigger_write(1);
		}
	}
	
	// --- new results are in! Do some stuff!
	if (event_id == EVENT_USE_RESULT) {
		// -- calc laps from hits
		info.calc_laps();
		info.print_heat();
		// -- write control message to uart
		uart.send_msg();
		// -- remember time
		//httpc.time_last_good_result = esp_timer_get_time();
	}
	
	// --- find rtsd hostname, if not known after last wifi connect
	if (!wifi.rtsd_ip_found) {
		wifi.rtsd_hostent = gethostbyname("rtsd");
		if (wifi.rtsd_hostent != NULL) {
			sprintf(wifi.rtsd_ip, "%d.%d.%d.%d", *(wifi.rtsd_hostent->h_addr)
												, *(wifi.rtsd_hostent->h_addr+1)
												, *(wifi.rtsd_hostent->h_addr+2)
												, *(wifi.rtsd_hostent->h_addr+3));
			ESP_LOGI(TAG, "gethostbyname rtsd '%s'", wifi.rtsd_ip);
			wifi.rtsd_ip_found = true;
		} else {
			ESP_LOGI(TAG, "gethostbyname rtsd failed '%s'", esp_err_to_name(h_errno));
		}
	}
	
	// --- scroll sad message if too long no good result
	int64_t time_last_good_delta = (esp_timer_get_time() - httpc.time_last_good_result) / 1000000;
	if (time_last_good_delta > 20) {
		//if (!timer.dont_print_sad) {
		char ctmp[256];
		//sprintf(ctmp, "RTsign scrolltext Test 21.10.2020 VfB Forever! Corona nervt gewaltig. 2021 hoffentlich wieder Rennen?");
		sprintf(ctmp, "RTsign sadly no see RTsd for %llu seconds :(", time_last_good_delta);
		ws.write_scrolltext(&ctmp[0], strlen(ctmp));
		
		//while(1) {
		//if (!timer.dont_print_sad) {
		//	timer.dont_print_sad = true;
		//	//vTaskDelay(1000 / portTICK_PERIOD_MS);
		//	int64_t ti_delta = esp_timer_get_time() / 1000000;
		//	printf("still running '%llu'\r\n", ti_delta);
		//}
	}
	
	// --- draw scroltlext
	//int64_t timer_start = esp_timer_get_time();
	//ws.matrix_draw_plasma();
	//ws.matrix_draw_scroll({ 0, 8, 0 });
	//int64_t timer_delta = esp_timer_get_time() - timer_start;
	//printf("p+s time %lluus\r\n", timer_delta);
	
	// --- rtc test
	//time_t rtc_time;
	//time(&rtc_time);
	//struct tm * timeinfo = localtime(&rtc_time);
	//timeinfo = localtime (&rtc_time);
	
	//printf("time '%02d:%02d:%02d'\r\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
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
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_USE_RESULT, main_event_handler, main_loop_handle, NULL));
	
	// --- wait some for stuff to start up
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	
	//while(1) {
	//	vTaskDelay(1000 / portTICK_PERIOD_MS);
	//	int64_t ti_delta = esp_timer_get_time() / 1000000;
	//	printf("still running '%llu'\r\n", ti_delta);
	//}
	
	// --- init sd card
	sd.init();
	
	// --- init config, read cfg files
	cfg.init();
	
	// --- init info
	info.init();
	
	// --- init rt spi
	rtspi.init();
	
	// --- init ws2812
	ws.init();
	
	// --- init timer
	timer.init();
	
	// --- init wifi
	wifi.init();
	
	// --- init dns server
	dnss.init();
	
	// --- init http
	httpc.init();
	
	// --- init uart
	uart.init();
}
