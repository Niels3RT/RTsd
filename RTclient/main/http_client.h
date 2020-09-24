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
		uint32_t t_etmp;
		
	private:
	
	// -- functions
	public:
		void init(void);
		char * find_data(char *tbuf);
		char * parse_hex2uint32(char *ptmp);
		uint8_t parse_csv_line(char *ptmp);
		void request_pilotinfo(void);
		bool parse_pilotinfo(char *tbuf);
		void request_raceinfo(void);
		bool parse_raceinfo(char *tbuf);
		void request_results(void);
		bool parse_results(char *tbuf);
		int create_socket(void);
		int send_request(int sock);
		int read_reply(int sock);
		void start_client_thread(void);
	private:
		
};

#define PORT					80

#define HTTP_REQ_PREFIX 		"GET "
#define HTTP_REQ_POST	 		" HTTP/1.1\r\nHost: 192.168.99.32\r\n\r\n"

#define HTTP_GET_PILOTINFO		"/pilotinfo.csv"
#define TYPE_GET_PILOTINFO		0
#define HTTP_GET_RACEINFO		"/raceinfo.csv"
#define TYPE_GET_RACEINFO		1
#define HTTP_GET_SESSIONINFO	"/sessioninfo.csv"
#define TYPE_GET_SESSIONINFO	2
#define HTTP_GET_RESULTS		"/results.csv"
#define TYPE_GET_RESULTS		3
