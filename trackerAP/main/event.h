// --- OO
class oo_Event {
	// -- vars
	public:
		uint16_t events_cnt;
		uint8_t pilots[CFG_MAX_PILOTS];
		st_pilot pilots_data[CFG_MAX_PILOTS];
		uint8_t pilots_all_cnt;
		uint8_t pilots_cnt;
		st_session sessions[32];
		uint8_t sessions_cnt;
		bool is_open;
		uint8_t mod_cnt;
		st_event current;
		st_event event_empty;
		uint8_t cnt_sess_type[3];
		st_result fastest_laps[CFG_MAX_PILOTS];
		st_result quali_time[CFG_MAX_PILOTS];
		st_result result_empty;
	private:
		
	// -- functions
	public:
		void init(void);
		void clear();
		void open(uint8_t nr);
		void close(void);
		void create_new(st_event * evtmp);
		void read_event_pilots(void);
		void write_event_pilots(void);
		void read_pilots(void);
		void collect_results(void);
		void sort_fastest_laps(uint8_t pcount);
		void sort_quali(uint8_t pcount);
		void clear_results(void);
	private:

};

#define CFG_HEAT_CHANNELS	"heat_channels"
#define CFG_QUALI_MODE		"quali_mode"
#define CFG_QUALI_LAPS		"quali_laps"
#define CFG_QUALI_OTIME		"quali_otime"
#define CFG_RACE_MODE		"race_mode"
#define CFG_RACE_LAPS		"race_laps"

#define DEFAULT_PILOT_1		"Pilot 1"
#define DEFAULT_PILOT_2		"Pilot 2"
#define DEFAULT_PILOT_3		"Pilot 3"
#define DEFAULT_PILOT_4		"Pilot 4"
#define DEFAULT_PILOT_5		"Pilot 5"
#define DEFAULT_PILOT_6		"Pilot 6"
#define DEFAULT_PILOT_7		"Pilot 7"
#define DEFAULT_PILOT_8		"Pilot 8"

#define QUALI_OVERTIME		0
#define QUALI_CONSEC_LAPS	1
