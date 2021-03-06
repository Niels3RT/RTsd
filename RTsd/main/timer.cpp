#include "main.h"

static void timer_main_callback(void* arg);

// ****** init Timer
void oo_Timer::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init timer");
	
	// --- some vars to default
	tick = 0;
	s_count = 0;
	
	// --- set rtc to some default time
	struct tm tm_set;
	tm_set.tm_sec = 0;
	tm_set.tm_min = 0;
	tm_set.tm_hour = 0;
	tm_set.tm_mday = 01;
	tm_set.tm_mon = 1;
	tm_set.tm_year = 120;
	tm_set.tm_isdst = 0;
	set_rtc(&tm_set, 0, 0);
	rtc_is_set = false;
	rtc_upd_count = 0;
	rtc_upd_delta_min = 0x7fffffffffffffff;
	
	// --- init main timer
	init_timer_main();
}

// ****** set rtc
void oo_Timer::set_rtc(struct tm * tm_set, int ms, int add_sec) {
	// --- convert to timeval
	struct timeval tv_test;
	tv_test.tv_sec = mktime(tm_set) + add_sec;
	tv_test.tv_usec = ms * 1000;
	
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

	// --- count and send 1s event
	if (timer.s_count < 10) {
		// -- increment counter
		timer.s_count++;
	} else {
		// -- reset counter
		timer.s_count = 0;
		// -- send event to main loop
		ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_TIMER_MAIN, NULL, 0, portMAX_DELAY));
	}
	
	// --- continue start?
	if ((timer.start_run) && (timer.start_tick == timer.tick)) {
		// -- handle start counter
		if (timer.start_cnt > 0) {
			// -- decrement start sequence timer
			timer.start_cnt--;
			// -- handle start timer
			timer.start_tick = timer.tick + 10;
			// -- send start squence event
			ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_START_RUN, NULL, 0, portMAX_DELAY));
		} else {
			// -- end start sequence
			timer.start_run = false;
		}
	}
	
	// --- start run?
	if (timer.start_begin) {
		// -- reset signal
		timer.start_begin = false;
		// -- handle heat start timer
		timer.start_run = true;
		timer.start_tick = timer.tick + 10;
		timer.start_cnt = 10;
		// -- send event to main loop
		ESP_ERROR_CHECK(esp_event_post_to(main_loop_handle, TRACKER_EVENTS, EVENT_RT_START_BEGIN, NULL, 0, portMAX_DELAY));
	}
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
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 1000000));		//1000ms
    ESP_LOGI(TAG, "Started timer main, time since boot: %lld us", esp_timer_get_time());
}
