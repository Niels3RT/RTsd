// --- OO
class oo_Timer {
	// -- vars
	public:
		volatile uint32_t tick;
		volatile uint8_t s_count;
		volatile bool start_begin;
		volatile bool start_run;
		volatile uint32_t start_tick;
		volatile uint8_t start_cnt;
		bool rtc_is_set;
		uint8_t rtc_upd_count;
		int64_t rtc_upd_delta_min;
	private:
		

	// -- functions
	public:
		void init(void);
		void set_rtc(struct tm * tm_set, int ms, int add_sec);
		void init_timer_main(void);
	private:

};
