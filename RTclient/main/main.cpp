#include "main.h"

// --- declare stuff
// -- objects
oo_WiFi wifi;
oo_Timer timer;
oo_HTTPC httpc;
oo_CFG cfg;

// -- handles
esp_event_loop_handle_t main_loop_handle;

ESP_EVENT_DEFINE_BASE(TRACKER_EVENTS);         // declaration of the tracker events family

// ****** main event handler
void main_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	//// --- do heat commit?
	//if (event_id == EVENT_RT_DO_COMMIT) {
	//	heat.commit();
	//}
	//
	// --- open heat?
	if (event_id == EVENT_TIMER_MAIN) {
		//httpc.start_request();
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
	
	// --- wait some for fpga to start up
	//vTaskDelay(5000 / portTICK_PERIOD_MS);
	
	// --- init config, read cfg files
	cfg.init();
	
	// --- init timer
	timer.init();
	
	// --- init wifi
	wifi.init();
	
	// --- init http
	httpc.init();
}
