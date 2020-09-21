#include "main.h"

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
		// -- 1 asc�i hex digit
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
		// -- 1 asc�i hex digit
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
