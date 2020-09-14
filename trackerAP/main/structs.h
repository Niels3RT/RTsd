// --- session
struct st_event {
	uint8_t nr;
	char name[40];
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
	uint16_t pilots_nr[4];
	uint8_t pilots_state[4];
	uint8_t lapcount[4];
	uint32_t fastest_laps_time[4];
	uint8_t fastest_laps_lapnr[4];
	uint8_t pos_nr[4];
	uint8_t pos_fastest_lap[4];
	uint32_t heat_time[4];
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
