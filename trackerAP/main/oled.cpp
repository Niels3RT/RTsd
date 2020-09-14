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

// ****** write framebuffer to bw oled
void oo_oLed::writefb(void) {
	// --- write fb
	rtspi.transmit_cmd(RT_WRITE_FB);
}

// ****** print hex, 8bit
void oo_oLed::print_hex_8u(uint8_t px, uint8_t py, uint8_t value) {
	uint8_t cmtp[2];
	buf.i82buf(&cmtp[0], value);
	print_char(px++, py, cmtp[0]);
	print_char(px, py, cmtp[1]);
}

// ****** print dec, 8bit
void oo_oLed::print_dec_8u(uint8_t px, uint8_t py, uint8_t value) {
	uint8_t cmtp[3];
	buf.i82buf_dec(&cmtp[0], value);
	print_char(px++, py, cmtp[0]);
	print_char(px++, py, cmtp[1]);
	print_char(px, py, cmtp[2]);
}

// ****** print hex, 16bit
void oo_oLed::print_hex_16u(uint8_t px, uint8_t py, uint16_t value) {
	uint8_t cmtp[4];
	buf.i162buf(&cmtp[0], value);
	print_char(px++, py, cmtp[0]);
	print_char(px++, py, cmtp[1]);
	print_char(px++, py, cmtp[2]);
	print_char(px, py, cmtp[3]);
}

// ****** print dec, 16bit
void oo_oLed::print_dec_16u(uint8_t px, uint8_t py, uint16_t value) {
	uint8_t cmtp[5];
	buf.i162buf_dec(&cmtp[0], value);
	print_char(px++, py, cmtp[0]);
	print_char(px++, py, cmtp[1]);
	print_char(px++, py, cmtp[2]);
	print_char(px++, py, cmtp[3]);
	print_char(px, py, cmtp[4]);
}

// ****** print hex, 32bit
void oo_oLed::print_hex_32u(uint8_t px, uint8_t py, uint32_t value) {
	uint8_t cmtp[7];
	buf.i322buf(&cmtp[0], value);
	print_char(px++, py, cmtp[0]);
	print_char(px++, py, cmtp[1]);
	print_char(px++, py, cmtp[2]);
	print_char(px++, py, cmtp[3]);
	print_char(px++, py, cmtp[4]);
	print_char(px++, py, cmtp[5]);
	print_char(px++, py, cmtp[6]);
	print_char(px, py, cmtp[7]);
}