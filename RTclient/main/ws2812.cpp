#include "main.h"
//                              0     1     2     3     4     5     6     7     8     9     -     blank
const char oo_WS::font_7s[] = { 0x3f, 0x03, 0x76, 0x67, 0x4b, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40, 0x00 };

// --- 256 byte array of sin
const char sinfield[256]={
0x80, 0x83, 0x86, 0x89, 0x8C, 0x8F, 0x92, 0x95, 0x98, 0x9B, 0x9E, 0xA2, 0xA5, 0xA7, 0xAA, 0xAD,
0xB0, 0xB3, 0xB6, 0xB9, 0xBC, 0xBE, 0xC1, 0xC4, 0xC6, 0xC9, 0xCB, 0xCE, 0xD0, 0xD3, 0xD5, 0xD7,
0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEB, 0xED, 0xEE, 0xF0, 0xF1, 0xF3, 0xF4,
0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD, 0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEE, 0xED, 0xEB, 0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
0xDA, 0xD7, 0xD5, 0xD3, 0xD0, 0xCE, 0xCB, 0xC9, 0xC6, 0xC4, 0xC1, 0xBE, 0xBC, 0xB9, 0xB6, 0xB3,
0xB0, 0xAD, 0xAA, 0xA7, 0xA5, 0xA2, 0x9E, 0x9B, 0x98, 0x95, 0x92, 0x8F, 0x8C, 0x89, 0x86, 0x83,
0x80, 0x7C, 0x79, 0x76, 0x73, 0x70, 0x6D, 0x6A, 0x67, 0x64, 0x61, 0x5D, 0x5A, 0x58, 0x55, 0x52,
0x4F, 0x4C, 0x49, 0x46, 0x43, 0x41, 0x3E, 0x3B, 0x39, 0x36, 0x34, 0x31, 0x2F, 0x2C, 0x2A, 0x28,
0x25, 0x23, 0x21, 0x1F, 0x1D, 0x1B, 0x19, 0x17, 0x15, 0x14, 0x12, 0x11, 0x0F, 0x0E, 0x0C, 0x0B,
0x0A, 0x09, 0x07, 0x06, 0x05, 0x05, 0x04, 0x03, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x09,
0x0A, 0x0B, 0x0C, 0x0E, 0x0F, 0x11, 0x12, 0x14, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F, 0x21, 0x23,
0x25, 0x28, 0x2A, 0x2C, 0x2F, 0x31, 0x34, 0x36, 0x39, 0x3B, 0x3E, 0x41, 0x43, 0x46, 0x49, 0x4C,
0x4F, 0x52, 0x55, 0x58, 0x5A, 0x5D, 0x61, 0x64, 0x67, 0x6A, 0x6D, 0x70, 0x73, 0x76, 0x79, 0x7C
};

// ****** init ws2812
void oo_WS::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init WS2812");

	// --- some vars to default
	tmp_cnt = 0;
	scroll_pos = 0;
	scroll_delta = 0;
	sprintf(scrolltext, "...............");
	scroll_len = strlen(scrolltext);
}

// ****** write string to scrolltext bram
void oo_WS::write_scrolltext(char * cptr, int clen) {
	char spi_buf[27];
	int rlen = clen;
	
	//printf("write scrolltext\r\n");

	// --- walk through text
	uint32_t adr = 0;
	int count = 0;
	int pos = 0;
	while(rlen != 0) {
		// -- determine number of bytes to send
		if (rlen > 24) {
			count = 24;
			rlen -= 24;
		} else {
			count = rlen;
			rlen = 0;
		}
		
		// -- copy string to spi buffer
		for (uint8_t i=0;i<count;i++) {
			spi_buf[i] = *(cptr++);
		}
	
		// -- calc adr
		adr = 0x0f;				// adr 14
		adr |= pos << 4;		// pos
		adr |= count << 16;		// char count
		
		spi_buf[24] = (adr >> 16) & 0xff;
		spi_buf[25] = (adr >> 8) & 0xff;
		spi_buf[26] = adr & 0xff;
		// -- send block of data
		rtspi.write216(&spi_buf[0]);
		// -- increment position counter
		pos += count;
	}
	
	// --- set scrolltext len
	rtspi.transmit16(SPI_MTX_TEXT_LEN, clen);
	
	// --- set scrolltext color
	rtspi.transmit24(SPI_MTX_TEXT_COLOR, 0x0f0000, 1);
	
	// --- set scrolltext line colors
	char * colptr = &spi_buf[0];
	char tmp = 0;
	for (uint8_t i=0;i<8;i++) {
		tmp = i * 2;
		// grb
		*(colptr++) = tmp;
		*(colptr++) = 14 - tmp;
		*(colptr++) = 0;
	}
	// -- calc adr
	adr = 0x0b;				// adr 0x0b
	spi_buf[24] = (adr >> 16) & 0xff;
	spi_buf[25] = (adr >> 8) & 0xff;
	spi_buf[26] = adr & 0xff;
	// -- send block of data
	rtspi.write216(&spi_buf[0]);
	
	// --- print color bar for default in vhd
	//for (uint8_t i=0;i<24;i++) {
	//	printf("%02x", spi_buf[i]);
	//}
	//printf("\r\n");
}

// ****** print 7segment 2 digitnumber to fb
void oo_WS::print_7s_2d(st_rgb color, uint8_t value) {
	if (value > 99) {
		ws.print_digit(1, color, 10);
		ws.print_digit(0, color, 10);
		return;
	}
	uint8_t tmp;
	tmp = value / 10;
	ws.print_digit(1, color, tmp);
	tmp = value - (tmp * 10);
	ws.print_digit(0, color, tmp);
}

// ****** print 7segment digit to fb
void oo_WS::print_digit(uint8_t fb_nr, st_rgb color, uint8_t digit) {
	st_rgb * fb_ptr;
	
	// --- sanity check
	if (digit > 11) digit = 11;
	
	// --- point pinter to used fb
	switch(fb_nr) {
		// -- fb strip a
		case 0:
			fb_ptr = &fb_ws_a[0];
			break;
		// -- fb strip b
		case 1:
			fb_ptr = &fb_ws_b[0];
			break;
		// -- default to fb strip a
		default:
			fb_ptr = &fb_ws_a[0];
			break;
	}
	
	// --- print 8 segments
	uint8_t dfont = font_7s[digit];
	st_rgb black = { 0, 0, 0 };
	for (uint8_t k=0;k<8;k++) {
		// -- 8 leds per segment
		for (uint8_t i=0;i<8;i++) {
			if (dfont & 0x01) {
				*fb_ptr = color;
			} else {
				*fb_ptr = black;
			}
			fb_ptr++;
		}
		// -- shift font byte
		dfont >>= 1;
	}
}

// ****** print char to wx2812 matrix
void oo_WS::matrix_print_char(uint8_t x, uint8_t y, st_rgb color, uint8_t digit) {
	// --- get pointers and other stufff
	const char * fptr = font.adrof(digit);
	st_rgb * mptr = &fb_ws_m[(x*8)];
	char ctmp = 0;
	uint8_t dir = 1;
	
	// --- correct for ws2812 matrix line direction
	if (x & 0x01) {
		dir = 0;
		mptr -= y;
	} else {
		mptr += y;
	}
	
	// --- get fb pointer and walk through font
	for (uint8_t i=0;i<4;i++) {
		// -- get font line
		ctmp = *fptr++;
		// -- walk through line in font
		for (uint8_t l=0;l<8;l++) {
			if (dir & 0x01) {
				if (ctmp & 0x01) {
					*mptr = color;
				}
				ctmp >>= 1;
			} else {
				if (ctmp & 0x80) {
					*mptr = color;
				}
				ctmp <<= 1;
			}
			mptr++;
		}
		dir++;
		// -- correct for y
		if (dir & 0x01) {
			mptr += 2*y;
		} else {
			mptr -= 2*y;
		}
	}
}

// ****** draw pixel
inline void oo_WS::matrix_draw_pixel(int x, int y, st_rgb color) {
	// --- sanity check for borders and draw pixel or not, 60000 pixel in 24785us, inline 16160us
	if ((x >= 0) && (x < 32) && (y >= 0) && (y < 8)) {
		if (x & 0x01) {
			fb_ws_m[(x<<3)+7-y] = color;	// x ok
		} else {
			fb_ws_m[(x<<3)+y] = color;	// ok
		}
	}
}

// ****** print char to wx2812 matrix, check for borders (scroll)
void oo_WS::matrix_print_char_scr(int x, int y, st_rgb color, uint8_t digit) {
	// --- get font pointer and other stufff
	const char * fptr = font.adrof(digit);
	int tmp_y = 0;
	char ctmp = 0;
	
	// --- get fb pointer and walk through font
	for (uint8_t i=0;i<4;i++) {
		// -- restart at y0
		tmp_y = y;
		// -- get font line
		ctmp = *fptr++;
		// -- walk through line in font
		for (uint8_t l=0;l<8;l++) {
			if (ctmp & 0x01) {
				matrix_draw_pixel(x, tmp_y, color);
			}
			tmp_y++;
			ctmp >>= 1;
		}
		x++;
	}
}

// ****** handle and draw scrolltext
void oo_WS::matrix_draw_scroll(st_rgb color) {
	// --- handle scroll position
	if (scroll_delta < 4) {
		scroll_delta++;
	} else {
		scroll_delta = 0;
		scroll_pos++;
	}
	
	// --- clear framebuffer
	memset(&fb_ws_m[0], 0, sizeof(fb_ws_m));
	
	// --- draw 8 chars from -4
	if (scroll_pos >= scroll_len) scroll_pos = 0;
	for (int i=0;i<8;i++) {
		if ((scroll_pos+i) < scroll_len) {
			matrix_print_char_scr((i-scroll_delta)+(i*4), 1, color, scrolltext[scroll_pos+i]);
		} else {
			matrix_print_char_scr((i-scroll_delta)+(i*4), 1, color, scrolltext[(scroll_pos+i)-scroll_len]);
		}
	}
	transfer_fb(15, 256);
	trigger_write(15);
}

// ****** draw plasma on ws2812 matrix
void oo_WS::matrix_draw_plasma(void) {
	uint8_t ar = 0;
	uint8_t br = 0;
	uint8_t ag = 0;
	uint8_t bg = 0;
	uint8_t ab = 0;
	uint8_t bb = 0;
	
	// --- rgb
	for (uint8_t i=0;i<32;i++) {
		for (uint8_t j=0;j<8;j++) {
			ar = sinfield[((j<<2)+count_r1)&0xff] >> 3;
			br = sinfield[((i<<1)+count_r2)&0xff] >> 3;
			ag = sinfield[((j<<2)+count_g1)&0xff] >> 3;
			bg = sinfield[(((32-i)<<2)+count_g2)&0xff] >> 3;
			ab = sinfield[(((8-j)<<1)+count_b1)&0xff] >> 3;
			bb = sinfield[((i<<2)+count_b2)&0xff] >> 3;
			matrix_draw_pixel(i, j, { char((ar + br) >> 2), char((ag + bg) >> 2), char((ab + bb) >> 2) });
		}
	}
	count_r1+=6;
	count_r2+=5;
	count_g1+=4;
	count_g2+=6;
	count_b1 += 5;
	count_b2 += 4;
}

// ****** trigger write fb to ws2812 strip
void oo_WS::trigger_write(uint8_t nr) {
	// --- write cmd to spi
	rtspi.transmit8(SPI_TRIGGER_FB2WS, nr);
}

// ****** transfer fb
void oo_WS::transfer_fb(uint8_t nr, int max) {
	char spi_buf[27];
	st_rgb * fb_ptr;
	// --- point pinter to used fb
	switch(nr) {
		// -- fb strip a
		case 0:
			fb_ptr = &fb_ws_a[0];
			break;
		// -- fb strip b
		case 1:
			fb_ptr = &fb_ws_b[0];
			break;
		// -- fb matrix
		case 15:
			fb_ptr = &fb_ws_m[0];
			break;
		// -- default to fb strip a
		default:
			fb_ptr = &fb_ws_a[0];
			break;
	}
	
	// --- walk through fb
	uint32_t adr = 0;
	uint32_t tmp = 0;
	int count = 8;
	for (uint8_t k=0;k<(max/8);k++) {
		// -- copy data to buffer
		for (uint8_t i=0;i<8;i++) {
			spi_buf[i*3  ] = fb_ptr->g;
			spi_buf[i*3+1] = fb_ptr->r;
			spi_buf[i*3+2] = fb_ptr->b;
			fb_ptr++;
		}
		// -- calc adr
		adr = nr & 0x0f;
		tmp = k * count;
		adr |= tmp << 4;
		adr |= count << 20;
		spi_buf[24] = (adr >> 16) & 0xff;
		spi_buf[25] = (adr >> 8) & 0xff;
		spi_buf[26] = adr & 0xff;
		// -- send block of data
		rtspi.write216(&spi_buf[0]);
	}
}
