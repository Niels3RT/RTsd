#include "main.h"

static void timer_main_callback(void* arg);

// ****** init Timer
void oo_Timer::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init Timer");
	
	// --- some vars to default
	tick = 0;
	dont_print_sad = false;
	
	// --- init main timer
	init_timer_main();
	
	// --- test rtc
	//time_t rtc_time;
	//struct tm * timeinfo;
	
	struct tm tm_set;
	tm_set.tm_sec = 0;
	tm_set.tm_min = 0;
	tm_set.tm_hour = 0;
	tm_set.tm_mday = 01;
	tm_set.tm_mon = 1;
	tm_set.tm_year = 120;
	tm_set.tm_isdst = 0;
	set_rtc(&tm_set);
	
	//time(&rtc_time);
	//rtc_time = 0;
	//tm_set = localtime (&rtc_time);
	//tm_set = localtime (NULL);
	
	//tm_set->tm_sec = 0;
	//tm_set->tm_min = 0;
	//tm_set->tm_hour = 0;
	//tm_set->tm_mday = 01;
	//tm_set->tm_mon = 1;
	//tm_set->tm_year = 120;
	//tm_set->tm_isdst = 0;
	
	//printf("time to set '%02d:%02d:%02d'\r\n", tm_set->tm_hour, tm_set->tm_min, tm_set->tm_sec);
	//
	//struct timeval tv_test;
	//tv_test.tv_sec = mktime(tm_set);
	//tv_test.tv_usec = 0;
	//
	//rtc_time = tv_test.tv_sec;
	//tm_set = localtime(&rtc_time);
	//printf("time to set (reverse test) '%02d:%02d:%02d'\r\n", tm_set->tm_hour, tm_set->tm_min, tm_set->tm_sec);
	//
	//timezone tz = { 0, 0 };
	//int rc=settimeofday(&tv_test, &tz);
	//if(rc==0) {
	//	printf("settimeofday() successful.\r\n");
	//} else {
	//	printf("settimeofday() failed '%d'\r\n",errno);
	//}
	//
	//time(&rtc_time);
	//timeinfo = localtime(&rtc_time);
	//
	//printf("time '%02d:%02d:%02d'\r\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	//// --- touch test file
	//char ctmp[128];
	//sprintf(ctmp, "/time_test.txt");
	//sd.data_file_open(ctmp, "w");
	//if (sd.data_file != NULL) {
	//	sprintf(sd.data_line, "time '%02d:%02d:%02d'\r\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	//	sd.data_file_writeline();
	//	sd.data_file_close();
	//}
}

// ****** set rtc
void oo_Timer::set_rtc(struct tm * tm_set) {
	// --- convert to timeval
	struct timeval tv_test;
	tv_test.tv_sec = mktime(tm_set);
	tv_test.tv_usec = 0;
	
	// --- set rtc
	timezone tz = { 0, 0 };
	int rc=settimeofday(&tv_test, &tz);
	if(rc==0) {
		ESP_LOGI(TAG, "set RTC to '%02d:%02d:%02d' OK", tm_set->tm_hour, tm_set->tm_min, tm_set->tm_sec);
	} else {
		ESP_LOGI(TAG, "set RTC to '%02d:%02d:%02d' failed '%s'", tm_set->tm_hour, tm_set->tm_min, tm_set->tm_sec, strerror(errno));
	}
}

// ****** timer main callback routine
static void timer_main_callback(void* arg) {
	// --- raise tick
	timer.tick++;

	// --- send event to main loop
	ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_TIMER_MAIN, NULL, 0, portMAX_DELAY));
}

// ****** init timer main
void oo_Timer::init_timer_main(void) {
	// --- create arguments
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &timer_main_callback,
            .arg = NULL,
			.dispatch_method = ESP_TIMER_TASK,
            .name = "timer_main"
    };
    esp_timer_handle_t timer_main;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &timer_main));
    
	// --- Start the timer
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 100000));	//100ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 1000000));	//1000ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 2000000));		//2000ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 3000000));		//3000ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 15000000));		//15000ms
    ESP_LOGI(TAG, "Started timer main, time since boot: %lld us", esp_timer_get_time());
}
