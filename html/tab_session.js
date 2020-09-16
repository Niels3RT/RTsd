	// -------------------------------------------------------------------------------------------------------------------------
	// results tab
	// -------------------------------------------------------------------------------------------------------------------------
	
	// ****** init session tab
	function init_tab_session() {
		// --- buttons
		p_btn_sess_list_up.style.color = '#000000';
		p_btn_sess_list_up.onclick = function() {
			if (chk_btn_active(p_btn_sess_list_up)) {
				if (sessions_list_offset > 0) {
					sessions_list_offset--;
				}
				draw_tab_sessions();
			}
		}
		p_btn_sess_list_down.style.color = '#000000';
		p_btn_sess_list_down.onclick = function() {
			if (chk_btn_active(p_btn_sess_list_down)) {
				if (sessions_list_offset < (sessions_count - 4)) {
					sessions_list_offset++;
				}
				draw_tab_sessions();
			}
		}
		p_btn_sess_open.style.color = '#000000';
		p_btn_sess_open.onclick = function() {
			if (chk_btn_active(p_btn_sess_open)) {
				heat_nr_current = 0;
				heat_mod_cnt = 0;
				//batch_cmd = [18, 19, 21, 13];
				batch_cmd = [18, 19, 21];
				batch_cnt = 0;
				batch_cnt_top = 3;
			}
		}
		p_btn_sess_new.style.color = '#000000';
		p_btn_sess_new.onclick = function() {
			if (chk_btn_active(p_btn_sess_new)) {
				batch_cmd = [27, 23, 16];
				batch_cnt = 0;
				batch_cnt_top = 3;
			}
		}
		p_btn_sess_close.style.color = '#000000';
		p_btn_sess_close.onclick = function() {
			if (chk_btn_active(p_btn_sess_close)) {
				comm_request = 24;
			}
		}
		
		// --- event list selector
		document.getElementById("t_sessions0").onclick = function() { sessions_list_selected = 0; }
		document.getElementById("t_sessions1").onclick = function() { sessions_list_selected = 1; }
		document.getElementById("t_sessions2").onclick = function() { sessions_list_selected = 2; }
		document.getElementById("t_sessions3").onclick = function() { sessions_list_selected = 3; }
	
		// --- add session results table elements
		for (var i = 0; i < 16; i++) {
			var t_div = document.getElementById("div_sess_results");
			// -- div for heat table
			var t_table_div = document.createElement("div");
			t_table_div.setAttribute("class", "cls_sess_res_table_div");
			t_div.appendChild(t_table_div);
			// --- table top,m for heat name
			var t_table_top = document.createElement("table");
			t_table_top.setAttribute("class", "cls_sess_res_table_table");
			t_table_div.appendChild(t_table_top);
			var t_tr_top = document.createElement("tr");
			t_table_top.appendChild(t_tr_top);
			var t_td_top = document.createElement("td");
			t_td_top.setAttribute("class", "cls_sess_res_table_top_t");
			t_td_top.setAttribute("id", "id_sessions_table_top_" + i);
			t_tr_top.appendChild(t_td_top);
			// -- table body, for data
			var t_table = document.createElement("table");
			t_table.setAttribute("class", "cls_sess_res_table_table");
			t_table_div.appendChild(t_table);
			for (var k = 0; k < 4; k++) {
				var t_tr = document.createElement("tr");
				t_table.appendChild(t_tr);
				for (var m = 0; m < 3; m++) {
					var t_td = document.createElement("td");
					var ik = i * 4 + k;
					t_td.setAttribute("class", "cls_sess_res_table_t" + m);
					t_td.setAttribute("id", "id_sessions_table_" + ik + "-" + m);
					t_tr.appendChild(t_td);
				}
			}
		}
		
		// --- handle session tables
		//window_resize();
		//window.dispatchEvent(new Event('resize'));
	}
	
	// ****** redraw tables n stuff on sessions tab
	function draw_tab_sessions() {
		// --- sessionss table
		for (var i = 0; i < 4; i++) {
			// --- table column 0
			document.getElementById("t_sessions" + i).textContent = sessions_names[i + sessions_list_offset];
		}
		
		// --- open/close buttons
		if (session_is_open) {
			if ((state != 0) && (state != 3)) {
				p_btn_sess_open.style.background = col_btn_inactive;
				p_btn_sess_new.style.background = col_btn_inactive;
				p_btn_sess_close.style.background = col_btn_inactive;
				p_btn_sess_list_up.style.background = col_btn_inactive;
				p_btn_sess_list_down.style.background = col_btn_inactive;
			} else {
				p_btn_sess_open.style.background = col_btn_inactive;
				p_btn_sess_new.style.background = col_btn_inactive;
				p_btn_sess_close.style.background = col_btn_active;
				p_btn_sess_list_up.style.background = col_btn_inactive;
				p_btn_sess_list_down.style.background = col_btn_inactive;
			}
		} else {
			if (event_is_open) {
				p_btn_sess_open.style.background = col_btn_active;
				p_btn_sess_new.style.background = col_btn_active;
				p_btn_sess_close.style.background = col_btn_inactive;
				p_btn_sess_list_up.style.background = col_btn_active;
				p_btn_sess_list_down.style.background = col_btn_active;
			} else {
				p_btn_sess_open.style.background = col_btn_inactive;
				p_btn_sess_new.style.background = col_btn_inactive;
				p_btn_sess_close.style.background = col_btn_inactive;
				p_btn_sess_list_up.style.background = col_btn_inactive;
				p_btn_sess_list_down.style.background = col_btn_inactive;
			}
		}
		
		// --- results table top
		var rtmp = 0;
		for (i = 0; i < 16; i++) {
		//for (i = 0; i < sessions_heats_count; i++) {
			if (i < sessions_heats_count) {
				rtmp = i + 1;
				document.getElementById("id_sessions_table_top_" + i).textContent = rtmp + " / " + sessions_heats_count;
			} else {
				document.getElementById("id_sessions_table_top_" + i).textContent = "---";
			}
		}
		
		// --- results table, pilot names
		//for (i = 0; i < pilots_event_count; i++) {
		for (i = 0; i < 64; i++) {
			if (i < pilots_session_count) {
				// -- pilot names
				document.getElementById("id_sessions_table_" + i + "-" + 0).textContent = pilots_names[pilots_session[i]];
			} else {
				document.getElementById("id_sessions_table_" + i + "-" + 0).textContent = "";
			}
		}
		
		// --- results table, results
		//for (i = 0; i < sessions_heats_count; i++) {
		for (i = 0; i < 16; i++) {
			for (c = 0; c < 4; c++) {
				if ((pilots_session[(i*4)+c] != 0xffff) && (((i*4)+c) < pilots_session_count)) {
					// -- lap count
					document.getElementById("id_sessions_table_" + ((i*4)+c) + "-" + 1).textContent = heat_lap_cnt[i][c];
					// -- heat time
					if (!isNaN(heat_time[i][c])) {
						document.getElementById("id_sessions_table_" + ((i*4)+c) + "-" + 2).textContent = laptime2string(heat_time[i][c]);
					} else {
						document.getElementById("id_sessions_table_" + ((i*4)+c) + "-" + 2).textContent = "-:--.---";
					}
				} else {
					// -- lap count
					document.getElementById("id_sessions_table_" + ((i*4)+c) + "-" + 1).textContent = "";
					// -- heat time
					document.getElementById("id_sessions_table_" + ((i*4)+c) + "-" + 2).textContent = "";
				}
			}
		}
	}

	// ****** handle session window size stuff
	function handle_size_session() {
		
	}
	