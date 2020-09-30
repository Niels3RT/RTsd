	var boot_cmd = [3, 12];
	var boot_cnt = 0;
	var count = 0;
	var count_refresh = 20;
	var count_err = 0;
	var timestamp = 0;
	var tstamp_rssi = 0;
	var state = 0;
	var comm_state = 0;
	var comm_request = 0;
	var comm_request_try = 0;
	var do_cal = false;
	var do_rssi = false;
	var use_event_mode = 0;
	var heat_is_open = false;
	var session_is_open = false;
	var event_is_open = false;
	var allow_pedit = false;
	var do_pedit = false;
	var pilot_selected = 0xffff;
	var pilot_event_selected = 0xffff;
	var show_tab = 0;
	var totalRows = 1;
	var cellsInRow = 8;
	var refreshID = setInterval(function(){ timerfunc(); }, 100);
	//var path = "http://192.168.97.111/";
	//var path = "http://192.168.4.1/";
	var path = "http://192.168.99.32/";
	var file = "results.csv";
	var exfile = "unwichtich";
	var pilots_names = [];
	var pilots_nr = [];
	var pilots_count = 0;
	//var pilots_event_nr = [0, 4, 3, 2, 1, 8, 9, 7, 12];
	//var pilots_event_count = 9;
	var pilots_event_nr = [];
	var pilots_event_count = 0;
	var pilots_session = [];
	var pilots_session_count = 9;
	var event_name = "";
	var session_name = "";
	var heat_name = "";
	var heat_pilots = [0, 1, 2, 3, 4, 5, 6, 7];
	var heat_nr_current = 0;
	var hits = [];
	hits[0] = new Array();
	hits[1] = new Array();
	hits[2] = new Array();
	hits[3] = new Array();
	hits[4] = new Array();
	hits[5] = new Array();
	hits[6] = new Array();
	hits[7] = new Array();
	var laps = [];
	laps[0] = new Array();
	laps[1] = new Array();
	laps[2] = new Array();
	laps[3] = new Array();
	laps[4] = new Array();
	laps[5] = new Array();
	laps[6] = new Array();
	laps[7] = new Array();
	var rssi = [];
	rssi[0] = new Array();
	rssi[1] = new Array();
	rssi[2] = new Array();
	rssi[3] = new Array();
	rssi[4] = new Array();
	rssi[5] = new Array();
	rssi[6] = new Array();
	rssi[7] = new Array();
	var exceptions = [];		// 0x0000000 6s, 0x1000000 12s, mask position 0x0ffffff
	exceptions[0] = new Array();
	exceptions[1] = new Array();
	exceptions[2] = new Array();
	exceptions[3] = new Array();
	exceptions[4] = new Array();
	exceptions[5] = new Array();
	exceptions[6] = new Array();
	exceptions[7] = new Array();
	var heat_lap_cnt = [];
	heat_lap_cnt[0] = new Array();
	heat_lap_cnt[1] = new Array();
	heat_lap_cnt[2] = new Array();
	heat_lap_cnt[3] = new Array();
	heat_lap_cnt[4] = new Array();
	heat_lap_cnt[5] = new Array();
	heat_lap_cnt[6] = new Array();
	heat_lap_cnt[7] = new Array();
	var heat_time = [];
	heat_time[0] = new Array();
	heat_time[1] = new Array();
	heat_time[2] = new Array();
	heat_time[3] = new Array();
	heat_time[4] = new Array();
	heat_time[5] = new Array();
	heat_time[6] = new Array();
	heat_time[7] = new Array();
	var heat_fastest_laps = [];
	heat_fastest_laps[0] = new Array();
	heat_fastest_laps[1] = new Array();
	heat_fastest_laps[2] = new Array();
	heat_fastest_laps[3] = new Array();
	heat_fastest_laps[4] = new Array();
	heat_fastest_laps[5] = new Array();
	heat_fastest_laps[6] = new Array();
	heat_fastest_laps[7] = new Array();
	var event_fastest_laps_cnt = 0;
	var event_fastest_laps = [];
	var event_fastest_laps_pilotnr = [];
	var event_quali_cnt = 0;
	var event_quali_laps = [];
	var event_quali_time = [];
	var event_quali_pilotnr = [];
	var event_result_show_type = 0;
	var event_result_cnt = 0;
	var event_result_table_size = 0;
	var ex_count = [0, 0, 0, 0, 0, 0, 0, 0];
	var ex_mod_cnt = 0;
	var ex_mod_cnt_new = 0;
	var event_mod_cnt = 0xff;
	var event_mod_cnt_new = 0;
	var session_mod_cnt = 0xff;
	var session_mod_cnt_new = 0;
	var heat_mod_cnt = 0xff;
	var heat_mod_cnt_new = 0;
	var events_names = [];
	var events_count = 0;
	var events_list_offset = 0;
	var events_list_selected = 0;
	var sessions_names = [];
	//var sessions_names = ['Apple', 'Banana', 'Egg', 'Karotte', 'Zitrone', 'Orange', 'Starfruit'];
	var sessions_count = 7;
	var sessions_list_offset = 0;
	var sessions_list_selected = 0;
	var sessions_heats_count = 0;
	var hit_count = [0, 0, 0, 0, 0, 0, 0, 0];
	var lap_count = [0, 0, 0, 0, 0, 0, 0, 0];
	var lap_last_max = [0, 0, 0, 0, 0, 0, 0, 0];
	var det_level = [0, 0, 0, 0, 0, 0, 0, 0];
	var det_level_mod = [0, 0, 0, 0, 0, 0, 0, 0];
	var rssi_min = [0, 0, 0, 0, 0, 0, 0, 0];
	var rssi_max = [42, 42, 42, 42, 42, 42, 42, 42];
	var pos_fastest = [0, 1, 2, 3, 4, 5, 6, 7];
	var pos_fastest_lapnr = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff];
	var pos_heat = [0, 1, 2, 3, 4, 5, 6, 7];
	var pos_heat_laps = [0, 0, 0, 0, 0, 0, 0, 0];
	var pos_heat_times = [0, 0, 0, 0, 0, 0, 0, 0];
	//var det_level_auto = true;
	var f_chnR  = [5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917];
	var f_chnLR = [5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621];
	var f_chnF  = [5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880];
	var cfg_band = [0, 0, 0, 0];
	var cfg_chn = [2, 3, 4, 5];
	var cfg_frq = [5732, 5769, 5806, 5843];
	var cfg_dead = 8;
	var cfg_percent = 74;
	var cfg_min = 1400;
	var cfg_filter = 5;
	var menu_target = 1;
	var edit_target = 0;
	var onlongtouch;
	var lt_timer;
	var lt_touchduration = 500;
	var lt_clickposX = 0;
	var lt_chn = 0;
	var lt_show_tmp = false;
	
	var max_chn = 4;
	var cfg_event_channels = 4;
	var cfg_event_quali_mode = 0;
	var cfg_event_quali_laps = 3;
	var cfg_event_quali_otime = 120;
	var cfg_event_race_mode = 0;
	var cfg_event_race_laps = 3;
	
	var col_btn_active = '#66f466';
	var col_btn_active_rgb = 'rgb(102, 244, 102)';
	var col_btn_inactive = '#f46666';
	var col_btn_inactive_rgb = 'rgb(244, 102, 102)';
	var col_btn_busy = '#f4f466';
	
	var event_table_pilots_all_size = 0;
	var event_table_pilots_event_size = 0;
	
	// --- rssi trace vars
	var g_startx = 0;
	var g_diffx = 0;
	var g_starty = 0;
	var g_diffy = 0;
	var g_drag = false;
	var g_len = 0;
	var g_width = 0;
	var g_pos = 0;
	var g_pos_drag = 0;
	var g_touch = false;
	var g_auto = true;
	var g_level = false;
	var g_chn = 0;
	
	// --- layout
	var win_size_x = 0;
	var win_size_y = 0;
	var font_size_main = 16;
	
	// ****** laptime to string
	function laptime2string(ttime) {
		var stmp = "---";
		var tmpmin = 0;
		while (ttime >= 60) {
			tmpmin++;
			ttime -= 60;
		}
		if (ttime >= 10) {
			stmp = "" + tmpmin + ":" + ttime.toFixed(3);
		} else {
			stmp = "" + tmpmin + ":0" + ttime.toFixed(3);
		}
		return(stmp);
	}

	// ****** check if button is active, return true if active
	function chk_btn_active(p_tmp) {
		if (p_tmp.style.background  == col_btn_active_rgb) {
			return(true);
		} else {
			return(false);
		}
	}
	