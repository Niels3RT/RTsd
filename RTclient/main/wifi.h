// --- OO
class oo_WiFi {
	// -- vars
	public:
		struct hostent *rtsd_hostent;
		char rtsd_ip[16];
		bool rtsd_ip_found;
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
