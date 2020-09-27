#include "main.h"

static void timer_main_callback(void* arg);

// ****** init Timer
void oo_Timer::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init timer");
	
	// --- some vars to default
	tick = 0;
	
	// --- init main timer
	init_timer_main();
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
    //ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 100000));	//100ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 1000000));	//1000ms
	ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 2000000));		//2000ms
	//ESP_ERROR_CHECK(esp_timer_start_periodic(timer_main, 3000000));		//3000ms
    ESP_LOGI(TAG, "Started timer main, time since boot: %lld us", esp_timer_get_time());
}
