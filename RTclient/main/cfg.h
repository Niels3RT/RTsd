// --- OO
class oo_CFG {
	// -- vars
	public:
		uint8_t max_chn;
	private:
		
	// -- functions
	public:
		void init(void);
	private:

};

#define CFG_WIFI_MODE		"wifi_mode"
#define CFG_WIFI_MODE_AP	"softAP"
#define CFG_WIFI_MODE_STA	"sta"
#define CFG_WIFI_AP_IP		"wifi_ap_ip"
#define CFG_WIFI_AP_SSID	"wifi_ap_ssid"
#define CFG_WIFI_AP_KEY		"wifi_ap_key"
#define CFG_WIFI_STA_SSID	"wifi_sta_ssid"
#define CFG_WIFI_STA_KEY	"wifi_sta_key"
