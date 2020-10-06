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
oo_SD sd;

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
	vTaskDelay(500 / portTICK_PERIOD_MS);
	
	// --- init sd card
	sd.init();
	
	// --- init config, read cfg files
	cfg.init();
	
	// --- init info
	info.init();
	
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
