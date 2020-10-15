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
		void transmit_cmd(uint8_t adr);
		void transmit8(uint8_t adr, uint8_t value);
		uint16_t transmit16(uint8_t adr, uint16_t value);
		uint32_t transmit24(uint8_t adr, uint32_t value, uint8_t cd);
		uint32_t transmit32(uint8_t adr, uint32_t value, uint8_t cd);
		void read64(char* in64);
		void write64(char* out64);
		void read128(char* in128);
		void write128(char* out128);
		void readn(char* inn, uint8_t count);
	private:
};

// *** spi interface for fpga
#define PIN_NUM_MISO 		25
#define PIN_NUM_MOSI 		23
#define PIN_NUM_CLK  		19
#define PIN_NUM_CS   		22
#define PIN_NUM_DC   		26

// *** addresses for rt over spi
#define RT_READ_STATE		0x00
#define RT_WRITE_STATE		0x01
#define RT_WRITE_COUNT		0x02
#define RT_TYPE				0x03
#define RT_TUNE_BASE		0x10
#define RT_TRG_LEVEL_BASE	0x18
#define RT_PD_DEADTIME		0x20
#define RT_PD_AUTO_FACTOR	0x21
#define RT_PD_AUTO_MIN		0x22
#define RT_CLR_MINMAX		0x23
#define RT_PD_START			0x24
#define RT_PD_CLEAR			0x25
#define RT_PD_EX_CLEAR		0x26
#define RT_PD_DET_FIXED		0x27
#define RT_PD_EXC_BASE		0x28
#define RT_MIN_BASE			0x30
#define RT_MAX_BASE			0x38
#define RT_NORM_BASE		0x40
#define RT_QUOT_BASE		0x48
#define RT_TRIGGER_BASE		0x50

#define RT_SDRAM2REG_RSSI	0x60
#define RT_PEAK2REG			0x61
#define RT_REG2SDRAM_RSSI	0x62
#define RT_REG2SAMPLE_TABLE	0x63
#define RT_PLAY_SAMPLE		0x64

//#define RT_SDRAM2REG		0x63
//#define RT_REG2SDRAM		0x64

// --- oled
#define RT_PRINT_CHAR		0x80
#define RT_WRITE_FB			0x81
#define RT_CLEAR_FB			0x82

#define RT_STATUS_PDREADY	0x01