// --- OO
class oo_RT_spi {
	// -- vars
	public:
		uint16_t spi_rx_data[16];
		uint8_t spi_rx_data_max;
		uint8_t spi_tx_cmd;

	private:

	// -- functions
	public:
		void init(void);
		void transmit8(uint8_t adr, uint8_t value);
		uint16_t transmit16(uint8_t adr, uint16_t value);
		uint32_t transmit24(uint8_t adr, uint32_t value, uint8_t cd);
		void write128(char* out128);
		void write216(char* out216);
	private:
};

// *** spi interface for fpga
#define PIN_NUM_MISO 		25
#define PIN_NUM_MOSI 		23
#define PIN_NUM_CLK  		19
#define PIN_NUM_CS   		22
#define PIN_NUM_DC   		26

// *** addresses for rt over spi
#define SPI_TRIGGER_FB2WS	0x11

#define SPI_MTX_TEXT_LEN	0x20
#define SPI_MTX_HZ_CNT_MAX	0x21
#define SPI_MTX_TEXT_COLOR	0x22
