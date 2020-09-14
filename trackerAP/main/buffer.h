// --- OO
class oo_Buf {

	// --- define stuff
	#define Ring_Size		16000

	public:
		void init(void);
		void copy(uint8_t *p_dst, uint8_t *p_src, uint8_t count);
		void i82buf(uint8_t *bufptr, uint8_t value);
		void i82buf_dec(uint8_t *bufptr, uint8_t value);
		void i82buf_dec2(uint8_t *bufptr, uint8_t value);
		void i122buf(uint8_t *bufptr, uint16_t value);
		void i162buf(uint8_t *bufptr, uint16_t value);
		void i162buf_dec(uint8_t *bufptr, uint16_t value);
		void i322buf(uint8_t *bufptr, uint32_t value);
		uint16_t buf2uint16_t(uint8_t *bufptr);
		uint32_t buf2uint32_t(uint8_t *bufptr);
		uint16_t buf2uint16_t_dec(uint8_t *bufptr);
		uint16_t buf2uintX_t_dec(char *bufptr);

	private:

};
