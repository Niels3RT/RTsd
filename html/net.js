	// -------------------------------------------------------------------------------------------------------------------------
	// schedule/network stuff
	// -------------------------------------------------------------------------------------------------------------------------
	// ****** timer function, handle connections/comands etc.
	function timerfunc() {
		// --- increment counter
		count++;
		var rssi_delta = timestamp - tstamp_rssi - 3;
		if (state == 0) rssi_delta = 0;
		if (rssi_delta > 80) rssi_delta = 80;
		var buf_post = "";
		// --- do stuff according to state
		switch(comm_state) {
			case 0:
				// -- new request?
				if (comm_request != 0) {
					comm_state = 1;
				} else {
					// -- handle comm refresh counter
					count_refresh++;
					// -- fetch rssi data?
					//if (rssi_delta > 18) {
					if ((rssi_delta > 14) && (do_rssi == true)) {
						comm_state = 1;
						comm_request = 5;
					} else {
						// -- fetch results?
						//if (count_refresh >= 12) {
						if (count_refresh >= 8) {
							if (do_cal != true) {
								// -- start results update
								comm_state = 1;
								comm_request = 1;
								count_refresh = 0;
								// -- fetch changed exceptions?
								if ((do_rssi == true) && (ex_mod_cnt != ex_mod_cnt_new)) {
									comm_request = 12;
								}
								// -- fetch changed heat stuff?
								if (heat_mod_cnt != heat_mod_cnt_new) {
									comm_request = 13;
								}
							} else {
								// -- start calibration min max update
								comm_state = 1;
								comm_request = 8;
								count_refresh = 0;
							}
						}
					}
					// -- do bootup/batch requests
					if (batch_cnt < batch_cnt_top) {
						comm_state = 1;
						comm_request = batch_cmd[batch_cnt];
						count_refresh = 0;
						batch_cnt++;
					}
				}
				break;
			case 1:		// do nothing, request is being started
				break;
			case 2:		// do nothing, request is running
				break;
			default:	// undefined state, set to 0
				comm_state = 0;
				break;
		}
		// --- start request or something?
		if (comm_state == 1) {
			comm_state = 2;
			var commFile = new XMLHttpRequest();
			//commFile.timeout = 400;
			commFile.timeout = 600;
			//commFile.timeout = 1000;
			// --- request results
			if (comm_request == 1) {
				commFile.open("GET", path + file, true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - handle response, decode results
								parse_results(commFile.responseText);
								break;
							default:
								comm_state = 0;
								comm_request = 1;
								count_err++;
								break;
						}
					}
				}
			}
			// --- request comand
			if (comm_request == 2) {
				commFile.open("GET", path + exfile, true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								// - split file into lines into ; seperated parts
								var Lines = commFile.responseText.split(/\r\n|\n/);
								var LFields = Lines[0].split(';');
								tmp_state = parseInt(LFields[1], 16);
								if ((LFields[0] == "Start Done") || (LFields[0] == "Clear Done") || (LFields[0] == "Stop Done") || (LFields[0] == "Commit Done")) {
									//window.alert("Start Done");
									state = tmp_state;
									switch_state(state);
									count_err = 0;
								} else {
									comm_request = 2;
									count_err++;
								}
								break;
							default:
								comm_state = 0;
								comm_request = 2;
								count_err++;
								break;
						}
					}
				}
			}
			// --- request config
			if (comm_request == 3) {
				commFile.open("GET", path + "cfg_get;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - split file into lines into ; seperated parts
								var Lines = commFile.responseText.split(/\r\n|\n/);
								// - max channels
								var LFields = Lines[1].split(';');
								max_chn = parseInt(LFields[0], 16);
								// - channel frequencies
								var LFields = Lines[2].split(';');
								for (var i = 0; i < max_chn; i++) {
									cfg_frq[i] = parseInt(LFields[i], 16);
									// - find channel for frequency
									for (var k = 0; k < 8; k++) {
										if (f_chnR[k] == cfg_frq[i]) {
											cfg_chn[i] = k;
										}
									}
								}
								// - detect parameters and rssi filter
								var LFields = Lines[3].split(';');
								cfg_dead	= parseInt(LFields[0], 16);
								cfg_percent = parseInt(LFields[1], 16);
								cfg_min		= parseInt(LFields[2], 16);
								cfg_filter	= parseInt(LFields[3], 16);
								// - update values in system tab
								update_sys();
								break;
							default:
								comm_state = 0;
								comm_request = 3;
								count_err++;
								break;
						}
					}
				}
			}
			// --- save config
			if (comm_request == 4) {
				var conf_string = "";
				// -- channel frequencies
				conf_string += cfg_frq[0].toString(16) + ";"
								+ cfg_frq[1].toString(16) + ";"
								+ cfg_frq[2].toString(16) + ";"
								+ cfg_frq[3].toString(16) + ";";
				// -- detect parameters
				conf_string += var24byte(cfg_dead) + ";"
								+ var24byte(cfg_percent) + ";"
								+ var24byte(cfg_min) + ";"
								+ var24byte(cfg_filter) + ";";
				conf_string += "\r\n";
				commFile.open("GET", path + "cfg_set;" + conf_string, true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 4;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get rssi data block
			if (comm_request == 5) {
				commFile.open("GET", path + "rssi_get;" + var28byte(tstamp_rssi) + ";" + var24byte(rssi_delta) + ";", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								if (state != 0) {
									var Lines = commFile.responseText.split(/\r\n|\n/);
									if (Lines.length > 1) {
										var LFields = Lines[0].split(';');
										tstamp_rssi += Lines.length-2;
										for (var i = 1; i < Lines.length; i++) {
											LFields = Lines[i].split(';');
											var ts = parseInt(LFields[0], 16);
											for (var k = 0; k < max_chn; k++) {
												rssi[k][ts] = parseInt(LFields[k+1], 16);
											}
										}
									}
								}
								draw_rssi();
								if (g_auto) translate_rssi(-(tstamp_rssi-1), true);
								break;
							default:
								comm_state = 0;
								comm_request = 5;
								count_err++;
								break;
						}
					}
				}
			}
			// --- set detection levels
			if (comm_request == 6) {
				switch(max_chn) {
					case 4:
						commFile.open("GET", path + "level_set;" + var24byte(det_level[0]) + ";"
																+ var24byte(det_level[1]) + ";"
																+ var24byte(det_level[2]) + ";"
																+ var24byte(det_level[3]) + ";", true);
						break;
					case 8:
						commFile.open("GET", path + "level_set;" + var24byte(det_level[0]) + ";"
																+ var24byte(det_level[1]) + ";"
																+ var24byte(det_level[2]) + ";"
																+ var24byte(det_level[3]) + ";"
																+ var24byte(det_level[4]) + ";"
																+ var24byte(det_level[5]) + ";"
																+ var24byte(det_level[6]) + ";"
																+ var24byte(det_level[7]) + ";", true);
						break;
				}
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 6;
								count_err++;
								break;
						}
					}
				}
			}
			// --- write exceptions
			if (comm_request == 7) {
				commFile.open("POST", path + "ex_set;", true);
				// -- walk through channels
				buf_post = "";
				for (var i = 0; i < max_chn; i++) {
					for (var k = 0; k < ex_count[i]; k++) {
						buf_post += i + ";" + var28byte(exceptions[i][k]) + "\r\n";
					}
				}
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 7;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get min max for calibration
			if (comm_request == 8) {
				commFile.open("GET", path + "minmax_get;", true);
				// -- walk through channels
				buf_post = "";
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - parse data
								var Lines = commFile.responseText.split(/\r\n|\n/);
									if (Lines.length > 1) {
										var LFields = Lines[0].split(';');
										for (var i = 0; i < Lines.length; i++) {
											LFields = Lines[i].split(';');
											var ntmp = parseInt(LFields[0], 16);
											rssi_min[ntmp] = parseInt(LFields[1], 16);
											rssi_max[ntmp] = parseInt(LFields[2], 16);
										}
									}
								break;
							default:
								comm_state = 0;
								comm_request = 8;
								count_err++;
								break;
						}
					}
				}
			}
			// --- start calibration
			if (comm_request == 9) {
				commFile.open("GET", path + "cmd_start_cal;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - handle response, decode results
								parse_results(commFile.responseText);
								break;
							default:
								comm_state = 0;
								comm_request = 9;
								count_err++;
								break;
						}
					}
				}
			}
			// --- stop calibration
			if (comm_request == 10) {
				commFile.open("GET", path + "cmd_stop_cal;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - handle response, decode results
								parse_results(commFile.responseText);
								break;
							default:
								comm_state = 0;
								comm_request = 10;
								count_err++;
								break;
						}
					}
				}
			}
			// --- save calibration data
			if (comm_request == 11) {
				commFile.open("GET", path + "cmd_save_cal;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								// - handle response, decode results
								parse_results(commFile.responseText);
								break;
							default:
								comm_state = 0;
								comm_request = 11;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get exceptions from tracker
			if (comm_request == 12) {
				commFile.open("GET", path + "ex_get;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								var chtmp = 0;
								for (var i = 0; i < max_chn; i++) {
									ex_count[i] = 0;
								}
								//if (Lines.length > 1) {
									for (var i = 0; i < Lines.length; i++) {
										if (Lines[i].length > 8) {
											LFields = Lines[i].split(';');
											var chtmp = parseInt(LFields[0], 16);
											if (chtmp < max_chn) {
												exceptions[chtmp][ex_count[chtmp]] = parseInt(LFields[1], 16);
												ex_count[chtmp]++;
											}
										} else {
											// -- get ex mod counter
											LFields = Lines[i].split(';');
											if (LFields.length == 2) {
												ex_mod_cnt = parseInt(LFields[0], 16);
												ex_mod_cnt_new = ex_mod_cnt;
												//window.alert("ex mod cnt " + ex_mod_cnt);
											}
										}
									}
								//}
								draw_rssi();
								break;
							default:
								comm_state = 0;
								comm_request = 12;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get race info from tracker
			if (comm_request == 13) {
				commFile.open("GET", path + "raceinfo.csv", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								if (Lines.length == 5) {
									// -- line 0, state
									// - state
									LFields = Lines[0].split(';');
									state = parseInt(LFields[0], 16);
									switch_state(state);
									if (state == 0) timestamp = 0;
									// - heat mod counter
									heat_mod_cnt = parseInt(LFields[1], 16);
									heat_mod_cnt_new = heat_mod_cnt;
									// -- line 1, event
									LFields = Lines[1].split(';');
									event_name = LFields[3];
									// -- line 2, session
									LFields = Lines[2].split(';');
									session_name = LFields[3];
									// -- line 3, heat
									LFields = Lines[3].split(';');
									heat_name = LFields[3];
									for (var i = 0; i < 4; i++) {
										heat_pilots[i] = parseInt(LFields[4+i], 16);
									}
									printLaps();
									//window.alert("Session Name '" + session_name + "' - Heat Name '" + heat_name + "' Pilot 2 '" + heat_pilots[2] + "'");
								}
								break;
							default:
								comm_state = 0;
								comm_request = 13;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get pilot info from tracker
			if (comm_request == 14) {
				commFile.open("GET", path + "pilotinfo.csv", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								pilots_count = 0;
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 3) {
										pilots_nr[pilots_count] = parseInt(LFields[0], 16);
										pilots_names[pilots_count] = LFields[1];
										pilots_count++;
										//window.alert("nrmtp '" + nrtmp + "' - Pilot Name '" + pilots_names[nrtmp] + "'");
									}
								}
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 14;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get list of available events from tracker
			if (comm_request == 15) {
				commFile.open("GET", path + "event_get_available;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								events_count = 0;
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 2) {
										events_names[events_count] = LFields[0];
										//window.alert(events_count + "' - Event Name '" + events_names[events_count] + "'");
										events_count++;
									}
								}
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 15;
								count_err++;
								break;
						}
					}
				}
			}
			// --- open event
			if (comm_request == 16) {
				commFile.open("GET", path + "event_open;" + (events_list_offset+events_list_selected) + ";", true);
				//commFile.open("GET", path + "event_open;" + events_names[(events_list_offset+events_list_selected)] + ";", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 16;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get session list for event
			if (comm_request == 17) {
				commFile.open("GET", path + "session_get_available;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								sessions_count = 0;
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 3) {
										sessions_names[sessions_count] = LFields[1];
										//window.alert(sessions_count + "' - Session Name '" + sessions_names[sessions_count] + "'");
										sessions_count++;
									}
								}
								draw_tab_sessions();
								break;
							default:
								comm_state = 0;
								comm_request = 17;
								count_err++;
								break;
						}
					}
				}
			}
			// --- open session
			if (comm_request == 18) {
				commFile.open("GET", path + "session_open;" + (sessions_list_offset+sessions_list_selected) + ";", true);
				//commFile.open("GET", path + "event_open;" + events_names[(events_list_offset+events_list_selected)] + ";", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								draw_tab_sessions();
								break;
							default:
								comm_state = 0;
								comm_request = 18;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get session pilots
			if (comm_request == 19) {
				commFile.open("GET", path + "session_get_pilots;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								pilots_session_count = 0;
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 4) {
										pilots_session[pilots_session_count] = parseInt(LFields[2], 16);
										//window.alert(pilots_session_count + "' - Pilot Nr '" + pilots_session[pilots_session_count] + "'");
										pilots_session_count++;
									}
								}
								sessions_heats_count = (pilots_session_count + 3) >> 2;
								//window.alert("pilots_session_count '" + pilots_session_count + "'");
								draw_tab_sessions();
								break;
							default:
								comm_state = 0;
								comm_request = 19;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get event pilots
			if (comm_request == 20) {
				commFile.open("GET", path + "event_get_pilots;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								pilots_event_count = 0;
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 3) {
										pilots_event_nr[pilots_event_count] = parseInt(LFields[1], 16);
										//window.alert(pilots_event_count + "' - Pilot Nr '" + pilots_event_nr[pilots_event_count] + "'");
										pilots_event_count++;
									}
								}
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 20;
								count_err++;
								break;
						}
					}
				}
			}
			// --- open heat
			if (comm_request == 21) {
				commFile.open("GET", path + "heat_open;" + (heat_nr_current) + ";", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								translate_rssi(0, true);
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 21;
								count_err++;
								break;
						}
					}
				}
			}
			// --- get session info from tracker
			if (comm_request == 22) {
				commFile.open("GET", path + "sessioninfo.csv", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								var Lines = commFile.responseText.split(/\r\n|\n/);
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									if (LFields.length == 6) {
										heat_lap_cnt[LFields[0]][LFields[1]] = parseInt(LFields[2], 16);
										heat_time[LFields[0]][LFields[1]] = parseInt(LFields[3], 16)/1000;
										heat_fastest_laps[LFields[0]][LFields[1]] = parseInt(LFields[4], 16)/1000;
									}
								}
								draw_tab_sessions();
								break;
							default:
								comm_state = 0;
								comm_request = 22;
								count_err++;
								break;
						}
					}
				}
			}
			// --- close event
			if (comm_request == 23) {
				commFile.open("GET", path + "event_close;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 23;
								count_err++;
								break;
						}
					}
				}
			}
			// --- close session
			if (comm_request == 24) {
				commFile.open("GET", path + "session_close;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 24;
								count_err++;
								break;
						}
					}
				}
			}
			// --- create new event
			if (comm_request == 25) {
				commFile.open("POST", path + "event_new;", true);
				// -- walk through event config data
				buf_post = "";
				buf_post += document.getElementById("txt_event_new_name").value + ";"
				buf_post += document.getElementById("sel_event_mod_qmode").value + ";";
				buf_post += document.getElementById("event_mod_nb_qlaps").value + ";";
				buf_post += document.getElementById("event_mod_nb_qotime").value + ";";
				buf_post += document.getElementById("sel_event_mod_rmode").value + ";";
				buf_post += document.getElementById("event_mod_nb_rlaps").value + ";";
				buf_post += document.getElementById("event_mod_nb_chn").value + ";";
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 25;
								count_err++;
								break;
						}
					}
				}
			}
			// --- write event pilots to tracker
			if (comm_request == 26) {
				commFile.open("POST", path + "event_set_pilots;", true);
				// -- walk through event config data
				buf_post = "";
				for (var i = 0; i < pilots_event_count; i++) {
					buf_post += pilots_event_nr[i] + ";";
				}
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 26;
								count_err++;
								break;
						}
					}
				}
			}
			// --- create new session
			if (comm_request == 27) {
				commFile.open("POST", path + "session_new;", true);
				// -- walk through event config data
				buf_post = "";
				if (document.getElementById("sess_new_rb_type_train").checked) buf_post += "0;";
				if (document.getElementById("sess_new_rb_type_quali").checked) buf_post += "1;";
				if (document.getElementById("sess_new_rb_type_race").checked) buf_post += "2;";
				if (document.getElementById("sess_new_rb_gen_all").checked) buf_post += "0;";
				if (document.getElementById("sess_new_rb_gen_quali").checked) buf_post += "1;";
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 27;
								count_err++;
								break;
						}
					}
				}
			}
			// --- event results
			if (comm_request == 28) {
				commFile.open("GET", path + "event_results;", true);
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								clear_event_results();
								var Lines = commFile.responseText.split(/\r\n|\n/);
								for (var i = 0; i < Lines.length; i++) {
									LFields = Lines[i].split(';');
									// --- fastest laps
									if ((LFields.length == 4) && (parseInt(LFields[0], 16)) == 0) {
										event_fastest_laps_pilotnr[event_fastest_laps_cnt] = parseInt(LFields[1], 16);
										if (LFields[2] != "ffffffff") {
											event_fastest_laps[event_fastest_laps_cnt] = parseInt(LFields[2], 16)/1000;
										} else {
											event_fastest_laps[event_fastest_laps_cnt] = parseInt(LFields[2], 16);
										}
										event_fastest_laps_cnt++;
									}
									// --- quali line
									if ((LFields.length == 5) && (parseInt(LFields[0], 16)) == 1) {
										event_quali_pilotnr[event_quali_cnt] = parseInt(LFields[1], 16);
										event_quali_laps[event_quali_cnt] = parseInt(LFields[2], 16);
										event_quali_time[event_quali_cnt] = parseInt(LFields[3], 16)/1000;
										event_quali_cnt++;
									}
								}
								draw_tab_event();
								break;
							default:
								comm_state = 0;
								comm_request = 28;
								count_err++;
								break;
						}
					}
				}
			}
			// --- modify event
			if (comm_request == 29) {
				commFile.open("POST", path + "event_mod;", true);
				// -- walk through event config data
				buf_post = "";
				buf_post += document.getElementById("txt_event_new_name").value + ";"
				buf_post += document.getElementById("sel_event_mod_qmode").value + ";";
				buf_post += document.getElementById("event_mod_nb_qlaps").value + ";";
				buf_post += document.getElementById("event_mod_nb_qotime").value + ";";
				buf_post += document.getElementById("sel_event_mod_rmode").value + ";";
				buf_post += document.getElementById("event_mod_nb_rlaps").value + ";";
				buf_post += document.getElementById("event_mod_nb_chn").value + ";";
				commFile.onreadystatechange = function () {
					// -- request completed?
					if(commFile.readyState == 4) {
						switch (commFile.status) {
							// - cmd is ok
							case 200:
								comm_state = 0;
								count_err = 0;
								break;
							default:
								comm_state = 0;
								comm_request = 25;
								count_err++;
								break;
						}
					}
				}
			}
			// --- common
			comm_request_try = comm_request;
			comm_request = 0;
			// --- do something in case of error
			commFile.onerror = function() {
				comm_state = 0;
				comm_request = comm_request_try;
				count_err++;
			}
			// --- show error?
			if (count_err > 5)	document.getElementById("div_header_m").textContent = "NOK";
			else				document.getElementById("div_header_m").textContent = "OK";
			// --- is post?
			if (buf_post.length > 2) {
				commFile.send(buf_post);
			}
			else						commFile.send(null);
		}
	}
	
	// ****** print var to 4 byte hex string
	function var24byte(value) {
		if (value == 0) return("0000");
		if (value < 0x0010) return("000" + value.toString(16));
		if (value < 0x0100) return("00" + value.toString(16));
		if (value < 0x1000) return("0" + value.toString(16));
		if (value > 0xffff) {
			return("ffff");
		} else {
			return(value.toString(16));
		}
	}
	
	// ****** print var to 8 byte hex string
	function var28byte(value) {
		if (value == 0) return("00000000");
		if (value < 0x00000010) return("0000000" + value.toString(16));
		if (value < 0x00000100) return("000000"  + value.toString(16));
		if (value < 0x00001000) return("00000"   + value.toString(16));
		if (value < 0x00010000) return("0000"    + value.toString(16));
		if (value < 0x00100000) return("000"     + value.toString(16));
		if (value < 0x01000000) return("00"      + value.toString(16));
		if (value < 0x10000000) return("0"       + value.toString(16));
		return(value.toString(16));
	}
	
	// ****** parse results, i.e. lap times
	function parse_results(responseText) {
		// --- split file into lines into ; seperated parts
		var Lines = responseText.split(/\r\n|\n/);
		var LFields = Lines[0].split(';');
		// --- read timestamp, do stuff with it
		timestamp = parseInt(LFields[0], 16);
		if (timestamp < tstamp_rssi) {
			tstamp_rssi = 0;
			//for (var i = 0; i < 4; i++) {
			//	lap_count[i] = 0;
			//	hit_count[i] = 0;
			//	laps[i][0] = "---";
			//}
		}
		// --- state
		state = parseInt(LFields[1], 16);
		switch_state(state);
		if (state == 0) timestamp = 0;
		// --- detection levels
		//if ((!g_drag) && (g_auto)) {		// prevent reset while dragging to change level
		if (!g_drag) {			// prevent reset while dragging to change level
			for (var i = 0; i < max_chn; i++) {
				det_level[i] = parseInt(LFields[i+2], 16);
				det_level_mod[i] = det_level[i];
			}
		}
		// --- exception mod counter
		//ex_mod_cnt_new = parseInt(LFields[6], 16);
		ex_mod_cnt_new = parseInt(LFields[max_chn+2], 16);
		// --- use event mode on controls?
		//var evm_tmp = parseInt(LFields[7], 16);
		var evm_tmp = parseInt(LFields[max_chn+3], 16);
		if (evm_tmp & 0x04) {
			heat_is_open = true;
		} else {
			heat_is_open = false;
		}
		if (evm_tmp & 0x02) {
			session_is_open = true;
		} else {
			session_is_open = false;
		}
		if (evm_tmp & 0x01) {
			event_is_open = true;
		} else {
			event_is_open = false;
		}
		if (use_event_mode != evm_tmp) {
			draw_tab_event();
			draw_tab_sessions();
		}
		use_event_mode = evm_tmp;
		// --- event mod counter
		//event_mod_cnt_new = parseInt(LFields[8], 16);
		event_mod_cnt_new = parseInt(LFields[max_chn+4], 16);
		if (event_mod_cnt != event_mod_cnt_new) {
			event_mod_cnt = event_mod_cnt_new;
			batch_cmd[batch_cnt_top] = 20;
			batch_cnt_top++;
			batch_cmd[batch_cnt_top] = 28;
			batch_cnt_top++;
		}
		// --- session mod counter
		//session_mod_cnt_new = parseInt(LFields[9], 16);
		session_mod_cnt_new = parseInt(LFields[max_chn+5], 16);
		if (session_mod_cnt != session_mod_cnt_new) {
			session_mod_cnt = session_mod_cnt_new;
			batch_cmd[batch_cnt_top] = 17;
			batch_cnt_top++;
			batch_cmd[batch_cnt_top] = 19;
			batch_cnt_top++;
			batch_cmd[batch_cnt_top] = 22;
			batch_cnt_top++;
			
		}
		// --- heat mod counter
		//heat_mod_cnt_new = parseInt(LFields[10], 16);
		heat_mod_cnt_new = parseInt(LFields[max_chn+6], 16);
		if (heat_mod_cnt != heat_mod_cnt_new) {
			heat_mod_cnt = heat_mod_cnt_new;
			count = 0;
			timestamp = 0;
			tstamp_rssi = 0;
			batch_cmd[batch_cnt_top] = 13;
			batch_cnt_top++;
		}
		// --- clear laptimes arrays
		for (var i = 0; i < max_chn; i++) {
			lap_count[i] = 0;
			hit_count[i] = 0;
			laps[i][0] = "---";
		}
		// --- read laptimes, only if state is running or stopped, ignore if ready to start
		//     seems dumb, but results and clean may be overlapping, keep it real
		if (state != 0) {
			var ts_old = 0;
			var ts = 0;
			var ltmp = 0;
			var chntmp = 99;
			var postmp = 99;
			var chnfield = 0;
			var count = 0;
			for (var i = 1; i < Lines.length; i++) {
				// -- split line into fields, fetch chn number
				LFields = Lines[i].split(';');
				// -- hits
				if (LFields[0] == "h") {
					chnfield = parseInt(LFields[1], 16);
					if (chnfield < max_chn) {
						// - check channel
						if (chntmp != chnfield) {
							chntmp = chnfield;
							ts_old = 0;
							ts = 0;
							ltmp = 0;
							lap_count[chntmp] = 0;
							hit_count[chntmp] = 0;
							count = 0;
						}
						// - calc laptimes
						ts = parseInt(LFields[3], 16);
						if ((count > 0) && (ts != 0)) {
							laps[chntmp][ltmp] = ts - ts_old;
							ltmp++;
							laps[chntmp][ltmp] = "---";
							lap_count[chntmp]++;
						}
						// - save hits for rssi graph
						if (ts != 0) {
							hits[chntmp][hit_count[chntmp]] = Math.floor(ts / 100);
							hit_count[chntmp]++;
						}
						ts_old = ts;
						count++;
					}
				}
				// -- fastest laps
				if (LFields[0] == "f") {
					postmp = parseInt(LFields[1], 16);
					pos_fastest[postmp] = parseInt(LFields[2], 16);
					pos_fastest_lapnr[postmp] = parseInt(LFields[3], 16);
				}
				// -- position
				if (LFields[0] == "p") {
					postmp = parseInt(LFields[1], 16);
					pos_heat[postmp] = parseInt(LFields[2], 16);
					pos_heat_laps[postmp] = parseInt(LFields[3], 16);
					pos_heat_times[postmp] = parseInt(LFields[4], 16);
				}
			}
		} else {
			lap_count = [0, 0, 0, 0, 0, 0, 0, 0];
			ex_count = [0, 0, 0, 0, 0, 0, 0, 0];
			for (var i = 0; i < max_chn; i++) {
				laps[i][0] = 0;
				lap_last_max[i] = 0;
				lap_count[i] = 0;
				hit_count[i] = 0;
				ex_count[i] = 0;
				pos_fastest[i] = i;
				pos_fastest_lapnr[i] = 0xff;
				pos_heat[i] = i;
				pos_heat_laps[i] = 0;
				pos_heat_times[i] = 0;
			}
			tstamp_rssi = 0;
			timestamp = 0;
		}
		printLaps();
		draw_rssi();
	}
