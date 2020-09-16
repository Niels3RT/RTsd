// --- OO
class oo_Session {
	// -- vars
	public:
		uint16_t nr;
		uint8_t mode;
		char name[40];
		st_heat heats[16];
		uint8_t heat_cnt;
		st_heat st_heat_empty;
		bool is_open;
		uint8_t mod_cnt;
	private:
		

	// -- functions
	public:
		void init(void);
		void clear(void);
		void open(uint8_t snr);
		void create_new(uint8_t sess_mode, uint8_t gen_mode);
		void gen_field(uint8_t gen_mode, uint8_t hcount);
		void close(void);
		void read_session_pilots(void);
	private:

};

#define CFG_SESSION_MODE		"mode"
#define CFG_SESSION_NAME		"name"

#define SESSION_MODE_TRAIN		0
#define SESSION_MODE_QUALI		1
#define SESSION_MODE_RACE		2

