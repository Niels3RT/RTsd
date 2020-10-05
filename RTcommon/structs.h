// --- event
struct st_event {
	uint8_t nr;
	char name[40];
	uint8_t channels;
	uint8_t quali_mode;
	uint8_t quali_laps;
	uint16_t quali_otime;
	uint8_t race_mode;
	uint8_t race_laps;
};

// --- heat
struct st_heat {
	uint8_t nr;
	char name[40];
	uint8_t state;
	uint16_t pilots_nr[8];
	uint8_t pilots_state[8];
	uint8_t lapcount[8];
	uint32_t fastest_laps_time[8];
	uint8_t fastest_laps_lapnr[8];
	uint8_t pos_nr[8];
	uint8_t pos_fastest_lap[8];
	uint32_t heat_time[8];
};

// --- session
struct st_session {
	uint8_t nr;
	char name[40];
	uint8_t mode;
};

// --- pilot
struct st_pilot {
	uint16_t nr;
	char name[40];
};

// --- result
struct st_result {
	uint16_t pilot_nr;
	uint8_t laps;
	uint32_t time;
};

// --- ip union
union union_ip {
	int ip32;
	char ip8x4[4];
};