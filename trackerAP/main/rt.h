// --- OO
class oo_RT {
	// -- vars
	public:
		volatile uint8_t state;
		volatile uint32_t count;
		uint16_t trg_level[4];
		uint16_t rssi_min[4];
		uint16_t rssi_max[4];
		float rssi_quot[4];
		uint16_t rssi_base[4];
		uint16_t chn_freq[4];
		uint32_t hits[4][256];
		uint8_t hitcount[4];
		uint32_t exceptions[4][32];
		uint8_t excount[4];
		uint8_t ex_mod_cnt;
		uint16_t deadtime;
		uint8_t det_quot_perc;
		uint16_t det_auto_min;
		bool do_calib;
	private:
	
	// -- functions
	public:
		void init(void);
		void get_state(void);
		void set_state(void);
		void start(void);
		void clear(bool keep_pilots);
		void stop(void);
		void set_count(void);
		void tune_rx_all();
		void tune_rx(uint8_t target_rx, uint16_t freq);
		void clear_minmax(void);
		void fetch_minmax(void);
		void write_cal_data(void);
		void pd_set_tlevel(uint8_t chn, uint16_t level);
		void pd_get_tlevels();
		void pd_set_deadtime(void);
		void pd_set_auto_min(void);
		void pd_set_fixed_mode(uint8_t mode);
		void pd_set_auto_quotient(void);
		void pd_set_exceptions(void);
		void pd_start(void);
		void pd_clear(void);
		bool pd_isready(void);
		void pd_fetch(void);
	private:
		
};
