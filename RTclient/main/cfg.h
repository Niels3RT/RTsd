// --- OO
class oo_CFG {
	// -- vars
	public:
		uint8_t max_chn;
		char wifi_sta_ssid[64];
		char wifi_sta_key[64];
	private:
		
	// -- functions
	public:
		void init(void);
		void read_wifi_cfg(void);
	private:

};

#define CFG_WIFI_STA_SSID		"wifi_sta_ssid"
#define CFG_WIFI_STA_KEY		"wifi_sta_key"

#define DEFAULT_WIFI_STA_SSID	"YourSSID"
#define DEFAULT_WIFI_STA_PASS	"YourPass"
