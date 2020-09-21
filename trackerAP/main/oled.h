// --- OO
class oo_oLed {
	// -- vars
	public:

	private:
		

	// -- functions
	public:
		void init(void);
		void print_char(uint8_t x, uint8_t y, uint8_t value);
		void print_string(uint8_t px, uint8_t py, char *tbuf);
		void writefb(void);
		void print_hex_8u(uint8_t px, uint8_t py, uint8_t value);
		void print_dec_8u(uint8_t px, uint8_t py, uint8_t value);
		void print_hex_16u(uint8_t px, uint8_t py, uint16_t value);
		void print_dec_16u(uint8_t px, uint8_t py, uint16_t value);
		void print_hex_32u(uint8_t px, uint8_t py, uint32_t value);
	private:
};
