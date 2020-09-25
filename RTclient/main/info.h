// --- OO
class oo_Info {
	// -- vars
	public:
		uint8_t state;
		uint32_t timestamp;
		st_pilot pilots_data[128];
		uint8_t pilot_count;
		uint8_t use_event_mode;
		st_event event;
		bool event_is_open;
		uint8_t event_mod_cnt;
		uint8_t event_mod_cnt_new;
		st_session session;
		bool session_is_open;
		uint8_t session_mod_cnt;
		uint8_t session_mod_cnt_new;
		st_heat heat;
		bool heat_is_open;
		uint8_t heat_mod_cnt;
		uint8_t heat_mod_cnt_new;
		
		uint32_t hits[8][256];
		uint8_t hitcount[8];
		uint32_t laps[8][256];
		uint8_t lapcount[8];
	private:
		

	// -- functions
	public:
		void init(void);
		void calc_laps(void);
		void print_heat(void);
	private:
		
};
