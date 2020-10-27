// --- OO
class oo_Timer {
	// -- vars
	public:
		volatile uint32_t tick;
		bool dont_print_sad;
	private:
		

	// -- functions
	public:
		void init(void);
		void set_rtc(struct tm * tm_set);
		void init_timer_main(void);
	private:

};
