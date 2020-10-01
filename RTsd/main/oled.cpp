#include "main.h"

// ****** init oled
void oo_oLed::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init oled");
}

// ****** print character to bw oled
void oo_oLed::print_char(uint8_t x, uint8_t y, uint8_t value) {
	// --- calc coordinates
	uint8_t postmp = ((x << 4) & 0xf0) + (y & 0x0f);
	
	// --- print char
	rtspi.transmit16(RT_PRINT_CHAR, (postmp<<8)+value);
}

// ****** print string
void oo_oLed::print_string(uint8_t px, uint8_t py, char *t_buf) {
	while(*t_buf != '\0') {
		print_char(px++, py, *(t_buf++));
	}
}

// ****** write framebuffer to bw oled
void oo_oLed::writefb(void) {
	// --- write fb
	rtspi.transmit_cmd(RT_WRITE_FB);
}

// ****** print hex, 8bit
void oo_oLed::print_hex_8u(uint8_t px, uint8_t py, uint8_t value) {
	char ctmp[3];
	sprintf(ctmp, "%02x", value);
	print_char(px++, py, ctmp[0]);
	print_char(px, py, ctmp[1]);
}

// ****** print dec, 8bit
void oo_oLed::print_dec_8u(uint8_t px, uint8_t py, uint8_t value) {
	char ctmp[4];
	sprintf(ctmp, "%02d", value);
	print_char(px++, py, ctmp[0]);
	print_char(px++, py, ctmp[1]);
	print_char(px, py, ctmp[2]);
}

// ****** print hex, 16bit
void oo_oLed::print_hex_16u(uint8_t px, uint8_t py, uint16_t value) {
	char ctmp[5];
	sprintf(ctmp, "%04x", value);
	print_char(px++, py, ctmp[0]);
	print_char(px++, py, ctmp[1]);
	print_char(px++, py, ctmp[2]);
	print_char(px, py, ctmp[3]);
}

// ****** print dec, 16bit
void oo_oLed::print_dec_16u(uint8_t px, uint8_t py, uint16_t value) {
	char ctmp[6];
	sprintf(ctmp, "%05d", value);
	print_char(px++, py, ctmp[0]);
	print_char(px++, py, ctmp[1]);
	print_char(px++, py, ctmp[2]);
	print_char(px++, py, ctmp[3]);
	print_char(px, py, ctmp[4]);
}

// ****** print hex, 32bit
void oo_oLed::print_hex_32u(uint8_t px, uint8_t py, uint32_t value) {
	char ctmp[9];
	sprintf(ctmp, "%08x", value);
	print_char(px++, py, ctmp[0]);
	print_char(px++, py, ctmp[1]);
	print_char(px++, py, ctmp[2]);
	print_char(px++, py, ctmp[3]);
	print_char(px++, py, ctmp[4]);
	print_char(px++, py, ctmp[5]);
	print_char(px++, py, ctmp[6]);
	print_char(px, py, ctmp[7]);
}
