// --- OO
class oo_Buf {

	public:
		void init(void);
		uint16_t buf2uint16_t(uint8_t *bufptr);
		uint32_t buf2uint32_t(uint8_t *bufptr);
		uint32_t parse_hex2uint32(char *ptmp);
		uint32_t parse_dec2uint32(char *ptmp);
		uint16_t buf2uint16_t_dec(uint8_t *bufptr);
		uint16_t buf2uintX_t_dec(char *bufptr);

	private:

};
