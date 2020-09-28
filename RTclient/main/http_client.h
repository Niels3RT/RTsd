// --- OO
class oo_HTTPC {
	// -- vars
	public:
		char rx_buf[4096];
		char tx_buf[16][1024];
		uint8_t msg_type[16];
		volatile uint8_t tx_buf_work;
		volatile uint8_t tx_buf_top;
		uint32_t csv_array[16];
		char csv_array_str[16][64];
		uint8_t csv_split_cnt;
		//uint32_t t_etmp;
		
	private:
	
	// -- functions
	public:
		void init(void);
		char * find_data(char *tbuf);
		uint32_t parse_hex2uint32(char *ptmp);
		char * split_csv_line(char *ptmp);
		void request_config(void);
		bool parse_config(char *tbuf);
		void request_pilotinfo(void);
		bool parse_pilotinfo(char *tbuf);
		void request_raceinfo(void);
		bool parse_raceinfo(char *tbuf);
		void request_results(void);
		bool parse_results(char *tbuf);
		void handle_mod_cnt(void);
		int create_socket(void);
		int send_request(int sock);
		int read_reply(int sock);
		void start_client_thread(void);
	private:
		
};

#define PORT					80

#define HTTP_REQ_PREFIX 		"GET "
#define HTTP_REQ_POST	 		" HTTP/1.1\r\nHost: 192.168.99.32\r\n\r\n"


#define HTTP_GET_CFG			"/cfg_get;"
#define TYPE_GET_CFG			0
#define HTTP_GET_PILOTINFO		"/pilotinfo.csv"
#define TYPE_GET_PILOTINFO		1
#define HTTP_GET_RACEINFO		"/raceinfo.csv"
#define TYPE_GET_RACEINFO		2
#define HTTP_GET_SESSIONINFO	"/sessioninfo.csv"
#define TYPE_GET_SESSIONINFO	3
#define HTTP_GET_RESULTS		"/results.csv"
#define TYPE_GET_RESULTS		4
