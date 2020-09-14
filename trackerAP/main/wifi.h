// --- OO
class oo_WiFi {
	// -- vars
	public:
		uint8_t ap_mode;
		char ap_ssid[40];
		char ap_key[40];
		char sta_ssid[40];
		char sta_key[40];
	private:
		

	// -- functions
	public:
		void init(void);
	private:
		void init_wifista(void);
		void init_softap(void);
};


//#define ESP_WIFI_SSID			"RTsd"
//#define ESP_WIFI_PASS			"peace2020"
#define ESP_WIFI_CHANNEL		11
#define MAX_STA_CONN			4

//#define ESP_WIFI_STA_SSID		"Sumpfnetz"
//#define ESP_WIFI_STA_PASS		"&zoBy#Iga6-8"
#define ESP_MAXIMUM_RETRY		3

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
