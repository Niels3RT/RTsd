// --- OO
class oo_Heat {
	// -- vars
	public:
		//uint8_t nr;
		//char name[40];
		//uint8_t pilots[4];
		uint32_t laps[4][256];
		st_heat current;
		//uint8_t lapcount[4];
		//uint32_t fastest_laps_time[4];
		//uint8_t fastest_laps_lapnr[4];
		//uint8_t pos_nr[4];
		//uint8_t pos_fastest_lap[4];
		//uint32_t heat_time[4];
		bool is_open;
		volatile uint8_t op_in_progress;
		uint8_t mod_cnt;
	private:
		
	// -- functions
	public:
		void init(void);
		void open(void);
		void clear(void);
		void commit(void);
		void calc_laps(void);
		void calc_position(void);
		void calc_fastest_laps(void);
	private:

};
