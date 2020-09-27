// --- OO
class oo_WiFi {
	// -- vars
	public:
		char sta_ssid[64];
		char sta_key[64];
	private:
		

	// -- functions
	public:
		void init(void);
	private:
		void init_wifista(void);
};

#define ESP_WIFI_CHANNEL		11
#define MAX_STA_CONN			4

#define ESP_MAXIMUM_RETRY		3

#define WIFI_CONNECTED_BIT 		BIT0
#define WIFI_FAIL_BIT      		BIT1

#define ESP_WIFI_STA_SSID		"Sumpfnetz"
#define ESP_WIFI_STA_PASS		"&zoBy#Iga6-8"
