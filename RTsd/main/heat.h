// --- OO
class oo_Heat {
	// -- vars
	public:
		uint32_t laps[8][256];
		st_heat current;
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
		void save_exceptions(void);
		void load_exceptions(void);
		void calc_laps(void);
		void calc_position(void);
		void calc_fastest_laps(void);
	private:

};
