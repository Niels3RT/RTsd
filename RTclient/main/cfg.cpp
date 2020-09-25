#include "main.h"

// ****** init config
void oo_CFG::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init config");
	
	// --- some vars to default
	max_chn = 4;
	
	// --- get config from tracker
	httpc.request_config();
}
