// --- OO
class oo_Timer {
	// -- vars
	public:
		volatile uint32_t tick;
		time_t rtc_time;
		struct tm * timeinfo;
	private:
		

	// -- functions
	public:
		void init(void);
		void init_timer_main(void);
	private:

};
