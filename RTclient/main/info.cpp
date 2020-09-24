#include "main.h"

// ****** init info
void oo_Info::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init Info");

	// --- some vars to default
	pilot_count = 0;
}
