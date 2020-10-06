#include "main.h"

// ****** init DNS server
void oo_DNSs::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init DNS server");
	
	// --- start dns server thread
	start_dnss_thread();
}

// ****** dns server thread
static void dns_server_task(void *pvParameters) {
    char rx_buffer[256];
	char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;

    while (1) {
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(53);
		ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "DNS Server Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "DNS Server Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "DNS Server Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "DNS Server Socket bound, port %d", 53);
		ESP_LOGI(TAG, "DNS Server, Waiting for requests.");

        while (1) {
			struct sockaddr_in source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
			
			// --- print info to serial log
			inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
			//ESP_LOGI(TAG, "DNS Server, request from '%s'", addr_str);

            // --- Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "DNS Server recvfrom failed: errno %d", errno);
                break;
			// --- request received
            } else {
				// -- print request data
				//uint8_t lcnt = 0;
				//char * rx_ptr = &rx_buffer[0];
				//for (int i=0;i<len;i++) {
				//	if (lcnt == 8) {
				//		lcnt = 1;
				//		printf("\r\n");
				//	} else {
				//		lcnt++;
				//	}
				//	printf("%02x ", *(rx_ptr++));
				//}
				//printf("\r\n");

				// -- check hostname
				bool reply_localhost = true;
				uint8_t hn_len = rx_buffer[12];
				if ((hn_len > 0) && (hn_len < 63)) {
					char hn_tmp[64];
					strncpy(hn_tmp, &rx_buffer[13], hn_len);
					hn_tmp[hn_len] = '\0';
					ESP_LOGI(TAG, "DNS Server, request from '%s' for '%s'", addr_str, hn_tmp);
					if ((hn_len == strlen(CFG_HOSTNAME)) && (strncmp(hn_tmp, CFG_HOSTNAME, strlen(CFG_HOSTNAME)) == 0)) reply_localhost = false;
				} else {
					ESP_LOGI(TAG, "DNS Server, malformed request from '%s'", addr_str);
					break;
				}
				 
				// -- parse request, modify into reply
				// - will be reply, set msb to 1, is authoritative answer bit 0x04
				rx_buffer[2] |= 0x84;
				// - walk request labels
				uint8_t pos = 12;	// set pointer to length of first label
				while(1) {
					if (rx_buffer[pos] != 0) {
						pos += rx_buffer[pos] + 1;
					} else {
						pos++;
						break;
					}
				}
				// - type
				int r_type = (rx_buffer[pos] << 8) + rx_buffer[pos+1];
				pos += 2;
				// - class
				int r_class = (rx_buffer[pos] << 8) + rx_buffer[pos+1];
				pos += 2;
				
				// -- is request type A (host address)?
				if ((r_type == 0x0001) && (r_class == 0x0001)) {
					// - add 1 answer record
					rx_buffer[6] = 0x00;
					rx_buffer[7] = 0x01;
					// -- add resource record for reply
					// - point to requested host name
					rx_buffer[pos++] = 0xc0;
					rx_buffer[pos++] = 0x0c;
					// - is type A, host address
					rx_buffer[pos++] = 0x00;
					rx_buffer[pos++] = 0x01;
					// - class IN
					rx_buffer[pos++] = 0x00;
					rx_buffer[pos++] = 0x01;
					// - time to live, 600s, 10 minutes
					rx_buffer[pos++] = 0x00;
					rx_buffer[pos++] = 0x00;
					rx_buffer[pos++] = 0x02;
					rx_buffer[pos++] = 0x58;
					// - data length, 4 octets of ip
					rx_buffer[pos++] = 0x00;
					rx_buffer[pos++] = 0x04;
					// - host ip address
					if (!reply_localhost) {
						// reply host ip
						rx_buffer[pos++] = dnss.host_ap_addr.ip8x4[0];
						rx_buffer[pos++] = dnss.host_ap_addr.ip8x4[1];
						rx_buffer[pos++] = dnss.host_ap_addr.ip8x4[2];
						rx_buffer[pos++] = dnss.host_ap_addr.ip8x4[3];
					} else {
						// reply localhost to divert requests not meant for us
						rx_buffer[pos++] = 127;
						rx_buffer[pos++] = 0;
						rx_buffer[pos++] = 0;
						rx_buffer[pos++] = 1;
					}
				} else {
					// - reply 'not implemented', at least try to play nice :P
					rx_buffer[3] = 0x04;
					// - don't write answer record
					rx_buffer[6] = 0x00;
					rx_buffer[7] = 0x00;
				}
				
				// -- send reply
                int err = sendto(sock, rx_buffer, pos, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
			// -- clear buffer
			memset(rx_buffer, 0, sizeof(rx_buffer));
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// ****** start request
void oo_DNSs::start_dnss_thread(void) {
	// --- start dns server thread
	xTaskCreate(dns_server_task, "DNS_server", 4096, (void*)AF_INET, 5, NULL);
}
