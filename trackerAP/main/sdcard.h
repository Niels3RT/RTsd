// --- OO
class oo_SD {
	// -- vars
	public:
		char cfg_parm[40];
		char cfg_value[40];
		char data_line[256];
		uint32_t csv_array[8];
		FILE* cfg_file;
		FILE* data_file;
		DIR *folder;
		struct dirent *entry;
	private:

	// -- functions
	public:
		void init(void);
		void cfg_file_open(char *tbuf, char const * rw);
		void data_file_open(const char *tbuf, char const * rw);
		void cfg_file_getparm();
		bool data_file_getline();
		bool data_file_getline_csv();
		void data_file_writeline(void);
		void cfg_file_writeline(void);
		void cfg_file_close();
		void data_file_close();
		bool dir_open(const char *tbuf);
		void dir_get_entry(void);
		void dir_close(void);
		void dir_create(char *tbuf);
	private:

};
