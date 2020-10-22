// --- OO
class oo_WS {
	// -- vars
	public:
		st_rgb fb_ws_a[64];
		st_rgb fb_ws_b[64];
		st_rgb fb_ws_m[256];
		char tmp_cnt;
		char scrolltext[512];
		int scroll_len;
		int scroll_pos;
		int scroll_delta;
		
		const static char font_7s[];
	private:
		// --- plasma positions in time
		uint8_t count_r1;
		uint8_t count_r2;
		uint8_t count_g1;
		uint8_t count_g2;
		uint8_t count_b1;
		uint8_t count_b2;
		
	// -- functions
	public:
		void init(void);
		void write_scrolltext(char * cptr, int clen);
		void print_7s_2d(st_rgb color, uint8_t value);
		void print_digit(uint8_t fb_nr, st_rgb color, uint8_t digit);
		void matrix_print_char(uint8_t x,uint8_t y, st_rgb color, uint8_t digit);
		inline void matrix_draw_pixel(int x, int y, st_rgb color);
		void matrix_print_char_scr(int x, int y, st_rgb color, uint8_t digit);
		void matrix_draw_scroll(st_rgb color);
		void matrix_draw_plasma(void);
		void trigger_write(uint8_t nr);
		void transfer_fb(uint8_t nr, int max);
	private:
		
};
