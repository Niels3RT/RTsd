// --- OO
class oo_UART {
	// -- vars
	public:
		char utx_buf[256];
	private:
		

	// -- functions
	public:
		void init(void);
		void add_crc(char *tbuf, uint8_t len);
		void send_msg(void);
	private:
		
};

#define ECHO_TEST_TXD  (GPIO_NUM_18)
#define ECHO_TEST_RXD  (GPIO_NUM_19)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)
