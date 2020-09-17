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
					sessions_list_selected++;
					session_table_session_select();
				}
				draw_tab_sessions();
			}
		}
		p_btn_sess_list_down.style.color = '#000000';
		p_btn_sess_list_down.onclick = function() {
			if (chk_btn_active(p_btn_sess_list_down)) {
				if (sessions_list_offset < (sessions_count - 4)) {
					sessions_list_offset++;
					sessions_list_selected--;
					session_table_session_select();
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
		
		// --- session list selector
		document.getElementById("t_sessions0").onclick = function() { sessions_list_selected = 0; session_table_session_select(); }
		document.getElementById("t_sessions1").onclick = function() { sessions_list_selected = 1; session_table_session_select(); }
		document.getElementById("t_sessions2").onclick = function() { sessions_list_selected = 2; session_table_session_select(); }
		document.getElementById("t_sessions3").onclick = function() { sessions_list_selected = 3; session_table_session_select(); }
	
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
		
		// --- handle session tables layout
		handle_layout_main();
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
	
	// ****** select session on session list
	function session_table_session_select() {
		for (var s = 0; s < 4; s++) {
			if (s == sessions_list_selected) {
				document.getElementById("t_sessions" + s).style.backgroundColor = '#666666';
			} else {
				document.getElementById("t_sessions" + s).style.backgroundColor = '#000000';
			}
		}
	}

	// ****** handle session window layout stuff
	function handle_layout_session() {
		var divsizeX = document.getElementById('div_sess_results').offsetWidth;
		var new_sizeX_table = (divsizeX / 4);
		var max_sizeX_table = 500;
		var min_sizeX_table = 250;
		var table_col_cnt = Math.floor(divsizeX / 4);
		var font_size = 1.8 + "vh";
		
		// --- keep the size real
		if (new_sizeX_table > max_sizeX_table) new_sizeX_table = max_sizeX_table;
		if (new_sizeX_table < min_sizeX_table) new_sizeX_table = min_sizeX_table;
		
		// --- make new size really fit
		table_col_cnt = Math.floor(divsizeX / new_sizeX_table);
		new_sizeX_table = divsizeX / table_col_cnt;
		new_sizeX_table = new_sizeX_table - 8;
		
		// --- font size
		font_size = (new_sizeX_table * 0.06) + "px";
		cell_height = (new_sizeX_table * 0.09) + "px";
		
		// --- single table div width
		var divs = document.getElementsByClassName("cls_sess_res_table_div");
		for (var i = 0; i < divs.length; i++) {
			divs[i].style.width = new_sizeX_table + "px";
		}
		// --- table top
		var tab_col_top = document.getElementsByClassName("cls_sess_res_table_top_t");
		for (i = 0; i < tab_col_top.length; i++) {
			tab_col_top[i].style.width = new_sizeX_table + "px";
			tab_col_top[i].style.height = cell_height;
			tab_col_top[i].style.fontSize = font_size;
			tab_col_top[i].style.textAlign = "left";
		}
		// --- table column 0
		var tab_col_0 = document.getElementsByClassName("cls_sess_res_table_t0");
		for (i = 0; i < tab_col_0.length; i++) {
			tab_col_0[i].style.width = (new_sizeX_table * 0.60) + "px";
			tab_col_0[i].style.height = cell_height;
			tab_col_0[i].style.fontSize = font_size;
			tab_col_0[i].style.textAlign = "left";
		}
		// --- table column 1
		var tab_col_1 = document.getElementsByClassName("cls_sess_res_table_t1");
		for (i = 0; i < tab_col_1.length; i++) {
			tab_col_1[i].style.width = (new_sizeX_table * 0.1) + "px";
			tab_col_1[i].style.fontSize = font_size;
			tab_col_1[i].style.textAlign = "center";
		}
		// --- table column 2
		var tab_col_2 = document.getElementsByClassName("cls_sess_res_table_t2");
		for (i = 0; i < tab_col_2.length; i++) {
			tab_col_2[i].style.width = (new_sizeX_table * 0.30) + "px";
			tab_col_2[i].style.fontSize = font_size;
			tab_col_2[i].style.textAlign = "right";
		}
		
		// --- session open div
		p_div_sess_open.style.fontSize = (font_size_main * 0.6) + "px";
	}
	