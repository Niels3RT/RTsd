#include "main.h"

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

// ****** init WiFi
void oo_WiFi::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init WiFi");
	switch(ap_mode) {
		case 0:
			// -- init soft AP
			init_softap();
			break;
		case 1:
			// -- init station
			init_wifista();
			break;
		default:
			// -- init soft AP
			init_softap();
			break;
	}
}

// ****** wifi event handler
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	// --- AP: station connected
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);

	// --- AP: station disconnected	
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
	
	// --- wifi station
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		char stmp[20];
		//sprintf(stmp, "%03d.%03d.%03d.%03d", (event->ip_info.ip.addr>>24) & 0xff, (event->ip_info.ip.addr>>16) & 0xff, (event->ip_info.ip.addr>>8) & 0xff, event->ip_info.ip.addr & 0xff);
		sprintf(stmp, "%03d.%03d.%03d.%03d", event->ip_info.ip.addr & 0xff, (event->ip_info.ip.addr>>8) & 0xff, (event->ip_info.ip.addr>>16) & 0xff, (event->ip_info.ip.addr>>24) & 0xff);
		oled.print_string(1, 7, &stmp[0]);
		oled.writefb();
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
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
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// --- register wifi event handler
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

	// --- config and start wifi
    wifi_config_t wifi_config = { };
	strcpy((char*)wifi_config.sta.ssid, sta_ssid);
	strcpy((char*)wifi_config.sta.password, sta_key);
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	wifi_config.sta.pmf_cfg.capable = true;
	wifi_config.sta.pmf_cfg.required = false;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    // --- xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID: %s password: %s",
                sta_ssid, sta_key);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s",
                sta_ssid, sta_key);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    // --- The event will not be processed after unregister
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

// ****** init soft AP
void oo_WiFi::init_softap(void) {
	// --- init netif
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

	// --- init wifi config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// --- register wifi handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

	// --- config and start wifi
    wifi_config_t wifi_config = { };
	strcpy((char*)wifi_config.ap.ssid, ap_ssid);
	wifi_config.ap.ssid_len = strlen(ap_ssid);
	wifi_config.ap.channel = ESP_WIFI_CHANNEL;
	strcpy((char*)wifi_config.ap.password, ap_key);
	wifi_config.ap.max_connection = MAX_STA_CONN,
	wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
	
	// --- config wifi country
	wifi_country_t country = {
		.cc = "DE",
        .schan = 1,
        .nchan = 13, 
        .max_tx_power = 78, 
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&country));
	ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(78));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	
	// --- print message to log
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ap_ssid, ap_key, ESP_WIFI_CHANNEL);
	
	// --- print ip info to oled
	tcpip_adapter_ip_info_t ipInfo; 
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
	//oled.print_hex_32u(0, 7, ipInfo.ip.addr);
	oled.print_dec_8u(12, 7, (ipInfo.ip.addr>>24) & 0xff);
	oled.print_char(11, 7, 0x2e);
	oled.print_dec_8u(8, 7, (ipInfo.ip.addr>>16) & 0xff);
	oled.print_char(7, 7, 0x2e);
	oled.print_dec_8u(4, 7, (ipInfo.ip.addr>>8) & 0xff);
	oled.print_char(3, 7, 0x2e);
	oled.print_dec_8u(0, 7, ipInfo.ip.addr & 0xff);
	oled.writefb();
}
