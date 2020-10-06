#include "main.h"

static EventGroupHandle_t s_wifi_event_group;

// ****** init WiFi
void oo_WiFi::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init WiFi");
	
	// --- set rtsd ip to default
	strcpy(rtsd_ip, "192.168.0.1");
	rtsd_ip_found = true;
	
	// --- default host ip address for dns server
	dnss.host_ap_addr.ip8x4[0] = 192;
	dnss.host_ap_addr.ip8x4[1] = 168;
	dnss.host_ap_addr.ip8x4[2] = 0;
	dnss.host_ap_addr.ip8x4[3] = 1;
	
	// --- init station
	init_wifista();
}

// ****** wifi event handler
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	// --- wifi station
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		vTaskDelay(6000 / portTICK_PERIOD_MS);
		ESP_LOGE(TAG, "WiFi disconnected, retry to connect to the AP");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
		ESP_LOGI(TAG, "WiFi connected to ap SSID: '%s' password: '%s'", cfg.wifi_sta_ssid, cfg.wifi_sta_key);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		// -- print host ip from dhcp
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "WiFi got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		// -- reset signal to request rtsd ip by hostname
		wifi.rtsd_ip_found = false;
		// -- remember host ip for dns server
		dnss.host_ap_addr.ip32 = event->ip_info.ip.addr;
		// -- print nameserver ip
		esp_netif_dns_info_t dns_info;
		esp_netif_get_dns_info(event->esp_netif, ESP_NETIF_DNS_MAIN, &dns_info);
		ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
    }
}

// ****** init station
void oo_WiFi::init_wifista(void) {
	s_wifi_event_group = xEventGroupCreate();

	// --- init netif
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

	// --- init wifi config
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

	// --- register wifi event handler
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

	// --- config and start wifi
    wifi_config_t wifi_config = { };
	strcpy((char*)wifi_config.sta.ssid, cfg.wifi_sta_ssid);
	strcpy((char*)wifi_config.sta.password, cfg.wifi_sta_key);
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	wifi_config.sta.pmf_cfg.capable = true;
	wifi_config.sta.pmf_cfg.required = false;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,CFG_HOSTNAME));
    ESP_ERROR_CHECK(esp_wifi_start() );
}
