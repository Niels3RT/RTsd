// --- OO
class oo_Audio {
	// -- vars
	public:
		uint32_t sample_adr;
	private:
		

	// -- functions
	public:
		void init(void);
		void play_sample(uint8_t nr);
		void sample_file_load(const char *sample_name, uint8_t sample_nr);
	private:
};

#define WAVE_GO				"go.wav"
#define WAVE_1				"1.wav"
#define WAVE_2				"2.wav"
#define WAVE_3				"3.wav"
#define WAVE_STOP			"stop.wav"
#define WAVE_GUN			"gun.wav"
#define WAVE_DING			"ding.wav"
#define WAVE_DONG			"dong.wav"
#define WAVE_N1				"N1.wav"
#define WAVE_N2				"N2.wav"
#define WAVE_N3				"N3.wav"
#define WAVE_N4				"N4.wav"
#define WAVE_N5				"N5.wav"
#define WAVE_N6				"N6.wav"
#define WAVE_N7				"N7.wav"
#define WAVE_N8				"N8.wav"
