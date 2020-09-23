// --- OO
class oo_HTTPC {
	// -- vars
	public:
		char rx_buf[16][4096];
		volatile uint8_t rx_buf_work;
		volatile uint8_t rx_buf_top;
		//uint16_t rxpos;
		char tx_buf[16][1024];
		volatile uint8_t tx_buf_work;
		volatile uint8_t tx_buf_top;
		//uint8_t tx_pos;
		
	private:
	
	// -- functions
	public:
		void init(void);
		void reply_done(char *tbuf);
		int create_socket(void);
		int send_request(int sock);
		int read_reply(int sock);
		void start_client_thread(void);
	private:
		
};

#define PORT					80
#define LISTENQ					8
#define MESSAGE 				"Hello World!"

#define HTTP_RX_BUFSIZE			4096
#define HTTP_TX_BUFSIZE			4096

#define HTTP_HEADER 			"HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: Keep-Alive\r\n"
#define HTTP_CONTENT_CSV		"Content-Type: text/csv\r\n\r\n"
#define HTTP_CONTENT_CSS		"Content-Type: text/css\r\n\r\n"
#define HTTP_CONTENT_JS			"Content-Type: text/javascript\r\n\r\n"
#define HTTP_CONTENT_HTML		"Content-Type: text/html\r\n\r\n"
#define HTTP_CONTENT_PNG		"Content-Type: image/png\r\n\r\n"
#define HTTP_CONTENT_JPG		"Content-Type: image/jpg\r\n\r\n"
#define HTTP_404 				"HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\nConnection: Keep-Alive\r\nContent-Type: text/csv\r\n\r\n"
#define HTTP_202 				"HTTP/1.1 202 Accepted\r\nAccess-Control-Allow-Origin: *\r\nConnection: Keep-Alive\r\nContent-Type: text/csv\r\n\r\n"
#define HTTP_DBG_CFG			"1664;1664;171D;1664;000B;004F;0514;0005;\r\n"

#define HTTP_GET 				"GET"
#define HTTP_POST 				"POST"
#define HTTP_GET_CFG 			"/cfg_get;"
#define HTTP_SET_CFG 			"/cfg_set;"
#define HTTP_GET_RESULTS		"/results.csv"
#define HTTP_GET_RACEINFO		"/raceinfo.csv"
#define HTTP_GET_SESSIONINFO	"/sessioninfo.csv"
#define HTTP_GET_PILOTINFO		"/pilotinfo.csv"
#define HTTP_GET_RSSI			"/rssi_get;"
#define HTTP_SET_TLEVEL			"/level_set;"
#define HTTP_SET_EXCEPT			"/ex_set;"
#define HTTP_GET_EXCEPT			"/ex_get;"
#define HTTP_GET_MINMAX			"/minmax_get;"
#define HTTP_GET_EVENTS			"/event_get_available;"
#define HTTP_GET_OPEN_EVENT		"/event_open;"
#define HTTP_SET_NEW_EVENT		"/event_new;"
#define HTTP_GET_EVENT_PILOTS	"/event_get_pilots;"
#define HTTP_SET_EVENT_PILOTS	"/event_set_pilots;"
#define HTTP_GET_EVENT_RESULTS	"/event_results;"
#define HTTP_GET_CLOSE_EVENT	"/event_close;"
#define HTTP_GET_SESSIONS		"/session_get_available;"
#define HTTP_GET_OPEN_SESSION	"/session_open;"
#define HTTP_GET_SESSION_PILOTS	"/session_get_pilots;"
#define HTTP_GET_CLOSE_SESSION	"/session_close;"
#define HTTP_GET_NEW_SESSION	"/session_new;"
#define HTTP_GET_OPEN_HEAT		"/heat_open;"
#define HTTP_CMD_START			"/cmd_start;"
#define HTTP_CMD_CLEAR			"cmd_clear;"
#define HTTP_CMD_COMMIT			"cmd_commit;"
#define HTTP_CMD_STOP			"cmd_stop;"
#define HTTP_CMD_START_CAL		"/cmd_start_cal;"
#define HTTP_CMD_SAVE_CAL		"/cmd_save_cal;"
#define HTTP_CMD_STOP_CAL		"/cmd_stop_cal;"
#define HTTP_SD_PATH			"/sdcard/RTsd/http"
#define HTTP_SD_PATH_EVENTS		"/RTsd/data"
