#include "main.h"

// ****** init buffer
void oo_Buf::init(void) {
	// --- write to log
	ESP_LOGI(TAG, "init buffer");

	return;
}

// ****** copy block from a to b
void oo_Buf::copy(uint8_t *p_dst, uint8_t *p_src, uint8_t count) {
	// --- crawl through given number of bytes
	for (uint8_t i=0;i<count;i++) {
		*p_dst++ = *p_src++;
	}
}

// ****** print´uint8_t to buffer at pos
void oo_Buf::i82buf(uint8_t *bufptr, uint8_t value) {
	uint8_t tmp = (value >> 4) & 0x0f;
	// --- upper nibble
	if (tmp > 0x09) {
		// -- 0xa0
		*bufptr++ = tmp + 0x37;
	} else {
		// -- 0xa0
		*bufptr++ = tmp + 0x30;
	}
	tmp = value & 0x0f;
	// --- lower nibble
	if (tmp > 0x09) {
		// -- 0xa0
		*bufptr++ = tmp + 0x37;
	} else {
		// -- 0xa0
		*bufptr++ = tmp + 0x30;
	}
}

// ****** print´uint8_t to buffer at pos, 3 digit decimal
void oo_Buf::i82buf_dec(uint8_t *bufptr, uint8_t value) {
	uint8_t tmpcount = 0;
		// -- 100
		while (value >= 100) {
			value -= 100;
			tmpcount++;
		}
		(*bufptr++) = (tmpcount + 0x0030);
		// -- 10
		tmpcount = 0;
		while (value >= 10) {
			value -= 10;
			tmpcount++;
		}
		(*bufptr++) = (tmpcount + 0x0030);
		// -- 1
		(*bufptr) = (value + 0x0030);
}

// ****** print´uint8_t to buffer at pos, 2digit decimal
void oo_Buf::i82buf_dec2(uint8_t *bufptr, uint8_t value) {
	uint8_t tmpcount = 0;
		// -- 10
		tmpcount = 0;
		while (value >= 10) {
			value -= 10;
			tmpcount++;
		}
		(*bufptr++) = (tmpcount + 0x0030);
		// -- 1
		(*bufptr) = (value + 0x0030);
}

// ****** print´uint16_t to buffer at pos, only lower 3 nibbles
void oo_Buf::i122buf(uint8_t *bufptr, uint16_t value) {
	uint16_t tmp = 0;
	value &= 0xfff;
	for (uint8_t i=0;i<3;i++) {
		tmp = (value >> 8) & 0x0f;
		// --- upper nibble
		if (tmp > 0x09) {
			// -- 0xa0
			*bufptr++ = tmp + 0x37;
		} else {
			// -- 0xa0
			*bufptr++ = tmp + 0x30;
		}
		value <<= 4;
	}
}

// ****** print´uint16_t to buffer at pos
void oo_Buf::i162buf(uint8_t *bufptr, uint16_t value) {
	uint16_t tmp = value;
	for (uint8_t i=0;i<4;i++) {
		tmp = (value >> 12) & 0x000f;
		// --- upper nibble
		if (tmp > 0x0009) {
			// -- 0xa0
			*(bufptr++) = tmp + 0x0037;
		} else {
			// -- 0xa0
			*(bufptr++) = tmp + 0x0030;
		}
		value <<= 4;
	}
}

// ****** print´uint16_t to buffer at pos, 5digit decimal
void oo_Buf::i162buf_dec(uint8_t *bufptr, uint16_t value) {
	uint8_t tmpcount = 0;
	// -- 10.000
	tmpcount = 0;
	while (value >= 10000) {
		value -= 10000;
		tmpcount++;
	}
	(*bufptr++) = (tmpcount + 0x0030);
	// -- 1.000
	tmpcount = 0;
	while (value >= 1000) {
		value -= 1000;
		tmpcount++;
	}
	(*bufptr++) = (tmpcount + 0x0030);
	// -- 100
	tmpcount = 0;
	while (value >= 100) {
		value -= 100;
		tmpcount++;
	}
	(*bufptr++) = (tmpcount + 0x0030);
	// -- 10
	tmpcount = 0;
	while (value >= 10) {
		value -= 10;
		tmpcount++;
	}
	(*bufptr++) = (tmpcount + 0x0030);
	// -- 1
	(*bufptr) = (value + 0x0030);
}

// ****** print uint32_t to buffer at pos
void oo_Buf::i322buf(uint8_t *bufptr, uint32_t value) {
	uint8_t tmp = 0;
	bufptr += 7;

	for (uint8_t i=0;i<8;i++) {
		tmp = value & 0x0000000f;
		// --- upper nibble
		if (tmp > 0x0009) {
			// -- 0xa0
			*(bufptr--) = tmp + 0x0037;
		} else {
			// -- 0xa0
			*(bufptr--) = tmp + 0x0030;
		}
		value >>= 4;
	}


}

// ****** convert 4 digit Ascii Hex to uint16
uint16_t oo_Buf::buf2uint16_t(uint8_t *bufptr) {
	uint16_t tmp = 0;
	uint8_t itmp = 0;
	// --- walk through digits
	for (uint8_t i=0;i<4;i++) {
		// -- rotate tmp left
		tmp <<= 4;
		// -- fetch byte
		itmp = *(bufptr++);
		// -- 1 ascíi hex digit
		if (itmp < 0x3a) {
			// - number
			tmp |= itmp - 0x30;
		} else {
			if (itmp < 0x60) {
				// - letter, big
				tmp |= itmp - 0x37;
			} else {
				// - letter, small
				tmp |= itmp - 0x57;
			}
		}
	}
	return(tmp);
}

// ****** convert 8 digit Ascii Hex to uint32
uint32_t oo_Buf::buf2uint32_t(uint8_t *bufptr) {
	uint32_t tmp = 0;
	uint8_t itmp = 0;
	// --- walk through digits
	for (uint8_t i=0;i<8;i++) {
		// -- rotate tmp left
		tmp <<= 4;
		// -- fetch byte
		itmp = *(bufptr++);
		// -- 1 ascíi hex digit
		if (itmp < 0x3a) {
			// - number
			tmp |= itmp - 0x30;
		} else {
			if (itmp < 0x60) {
				// - letter, big
				tmp |= itmp - 0x37;
			} else {
				// - letter, small
				tmp |= itmp - 0x57;
			}
		}
	}
	return(tmp);
}

// ****** convert 4 digit Ascii Decimal to uint16
uint16_t oo_Buf::buf2uint16_t_dec(uint8_t *bufptr) {
	uint16_t tmp = 0;
	// --- walk through digits
	tmp = ((uint16_t)(*bufptr++) - 0x30) * 1000;
	tmp += ((uint16_t)(*bufptr++) - 0x30) * 100;
	tmp += ((uint16_t)(*bufptr++) - 0x30) * 10;
	tmp += ((uint16_t)(*bufptr) - 0x30);
	return(tmp);
}

// ****** convert x digit Ascii Decimal to uint16
uint16_t oo_Buf::buf2uintX_t_dec(char *bufptr) {
	uint16_t etmp = 0;
	while((*bufptr >= 0x30) && (*bufptr < 0x3a)) {
		etmp *= 10;
		etmp += *bufptr - 0x30;
		bufptr++;
	}
	return(etmp);
}
