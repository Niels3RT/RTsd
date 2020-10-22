#include "main.h"

// --- declare stuff
// -- objects
oo_WiFi wifi;
oo_RT_spi rtspi;
oo_Timer timer;
oo_oLed oled;
oo_Audio audio;
oo_Buf buf;
oo_HTTP http;
oo_DNSs dnss;
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
	// --- open heat?
	} else if (event_id == EVENT_RT_DO_OPEN) {
		heat.open();
	// --- begin heat start?
	} else if (event_id == EVENT_RT_START_BEGIN) {
		int64_t timer_delta = esp_timer_get_time() / 1000;
		printf("Start Heat, begin '%d' at '%llu'\r\n", timer.start_cnt, timer_delta);
		rt.start();
		rt.get_state();
	// --- continue heat start?
	} else if (event_id == EVENT_RT_START_RUN) {
		int64_t timer_delta = esp_timer_get_time() / 1000;
		printf("Start Heat, sequence '%d' at '%llu'\r\n", timer.start_cnt, timer_delta);
		// -- play countdown?
		switch(timer.start_cnt) {
			case 3:
				audio.play_sample(3);
				break;
			case 2:
				audio.play_sample(2);
				break;
			case 1:
				audio.play_sample(1);
				break;
			case 0:
				audio.play_sample(0);
				break;
		}
	} else {
		//printf("Timer Event :)\r\n");
		// --- play audio test
		//audio.play_sample(4);
		
		//// --- rtc test
		//// -
		//int64_t timer_delta = esp_timer_get_time() / 1000;
		//// -
		//time_t rtc_time;
		//time(&rtc_time);
		//printf("time_t: '%lu' delta '%llu'\r\n", rtc_time, timer_delta);
		//struct tm * timeinfo = localtime(&rtc_time);
		//printf("rtc says '%04d:%02d:%02d-%02d:%02d:%02d'\r\n", timeinfo->tm_year + 1900, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	}
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
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_RT_START_BEGIN, main_event_handler, main_loop_handle, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register_with(main_loop_handle, TRACKER_EVENTS, EVENT_RT_START_RUN, main_event_handler, main_loop_handle, NULL));
	
	// --- wait some for fpga to start up
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	
	// --- init sd card
	sd.init();
	
	// --- init config, read cfg files
	cfg.init();
	
	// --- init rt spi
	rtspi.init();
	
	// --- init rt
	rt.init();
	
	// --- init audio
	audio.init();
	
	//while(1) {
	//	for (uint8_t i=0;i<16;i++) {
	//		// --- play audio test
	//		audio.play_sample(i);
	//		vTaskDelay(1000 / portTICK_PERIOD_MS);
	//	}
	//}
	
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
	
	// --- init dns server
	dnss.init();
	
	// --- init http
	http.init();
}
