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
		void readn(char* inn, uint8_t count);
		//void rt_spi_pre_transfer_callback(spi_transaction_t *t);
		//void rt_spi_post_transfer_callback(spi_transaction_t *t);
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
#define RT_WRITE_STATE		0xF0
#define RT_WRITE_COUNT		0x05
#define RT_WRITE_FB			0x01
#define RT_PRINT_CHAR		0x81
#define RT_SDRAM2REG		0xa0
#define RT_PEAK2REG			0xa1
#define RT_REG2SDRAM		0xa2
#define RT_TUNE_RX0			0x90
#define RT_TUNE_RX1			0x91
#define RT_TUNE_RX2			0x92
#define RT_TUNE_RX3			0x93
#define RT_TRG_LEVEL0		0x94
#define RT_TRG_LEVEL1		0x95
#define RT_TRG_LEVEL2		0x96
#define RT_TRG_LEVEL3		0x97
#define RT_PD_DEADTIME		0x98
#define RT_PD_AUTO_FACTOR	0x99
#define RT_PD_AUTO_MIN		0x9a
#define RT_CLR_MINMAX		0x10
#define RT_MIN_BASE			0x11
#define RT_MAX_BASE			0x15
#define RT_NORM_BASE		0x70
#define RT_QUOT_BASE		0x74
#define RT_TRIGGER_BASE		0x78
#define RT_PD_EXC_BASE		0x7c
#define RT_PD_START			0x02
#define RT_PD_CLEAR			0x03
#define RT_PD_EX_CLEAR		0x04
#define RT_PD_DET_FIXED		0xF1

#define RT_STATUS_PDREADY	0x01