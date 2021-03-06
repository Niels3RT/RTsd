// --- include some stuff only once
#ifndef tracker_H
#define tracker_H
	// --- this does not need to be included frome every .c
	
	// -- rgb struct for ws strips
	struct st_rgb {
		char r;
		char g;
		char b;
	};
	
	// -- esp libraries
	#include <string.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "freertos/event_groups.h"
	#include "esp_system.h"
	#include "esp_wifi.h"
	#include "esp_event.h"
	#include "esp_event_base.h"
	#include "esp_log.h"
	#include "esp_vfs_fat.h"
	#include "dirent.h"
	#include "nvs_flash.h"
	//#include <errno.h>
	
	#include "driver/spi_master.h"
	#include "driver/gpio.h"
	#include "driver/uart.h"
	#include "driver/sdmmc_host.h"
	#include "sdmmc_cmd.h"
	
	#include "lwip/err.h"
	#include "lwip/sockets.h"
	#include "lwip/sys.h"
	#include <lwip/netdb.h>
	
	// --- maxima etc
	#define CFG_MAX_PILOTS		64

	// --- homegrown stuff
	#include "../../RTcommon/structs.h"
	#include "wifi.h"
	#include "timer.h"
	#include "http_client.h"
	#include "cfg.h"
	#include "info.h"
	#include "uart.h"
	#include "rt_spi.h"
	#include "../../RTcommon/sdcard.h"
	#include "../../RTcommon/dns_server.h"
	#include "ws2812.h"
	#include "./font/font.h"

	// --- objects
	extern oo_WiFi wifi;
	extern oo_Timer timer;
	extern oo_HTTPC httpc;
	extern oo_DNSs dnss;
	extern oo_CFG cfg;
	extern oo_Info info;
	extern oo_UART uart;
	extern oo_RT_spi rtspi;
	extern oo_SD sd;
	extern oo_WS ws;
	extern oo_Font font;
	
	extern "C" {
		void app_main();
	}
	
	#define min(a,b) ((a)<(b)?(a):(b))

	// --- defaults
	static const char *TAG = "RTclient";
	
	// --- handles
	extern esp_event_loop_handle_t main_loop_handle;
	
	ESP_EVENT_DECLARE_BASE(TRACKER_EVENTS);         // declaration of the tracker events family

	enum {
		EVENT_TIMER_MAIN,	                    	 // raised during an iteration of the loop within the task
		EVENT_USE_RESULT	                    	 // raised when new results are in
	};
#endif
