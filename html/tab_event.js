	// -------------------------------------------------------------------------------------------------------------------------
	// events tab
	// -------------------------------------------------------------------------------------------------------------------------
	
	// ****** init event tab
	function init_tab_event() {
		// --- buttons
		document.getElementById("btn_event_list_up").onclick = function() {
			if (events_list_offset > 0) {
				events_list_offset--;
			}
			draw_tab_event();
		}
		document.getElementById("btn_event_list_down").onclick = function() {
			if (events_list_offset < (events_count - 4)) {
				events_list_offset++;
			}
			draw_tab_event();
		}
		p_event_btn_open.style.color = '#000000';
		p_event_btn_open.onclick = function() {
			//window.alert("Background Color '" + p_event_btn_open.style.background + "'");
			if (chk_btn_active(p_event_btn_open)) {
				p_event_btn_open.style.background = col_btn_busy;
				//comm_request = 16;
				batch_cmd = [16, 17, 20, 28];
				batch_cnt = 0;
				batch_cnt_top = 4;
			}
		}
		p_event_btn_close.style.color = '#000000';
		p_event_btn_close.onclick = function() {
			if (chk_btn_active(p_event_btn_close)) {
				p_event_btn_close.style.background = col_btn_busy;
				//comm_request = 23;
				batch_cmd = [23, 28];
				batch_cnt = 0;
				batch_cnt_top = 2;
			}
		}
		p_event_btn_new.style.color = '#000000';
		p_event_btn_new.onclick = function() {
			if (chk_btn_active(p_event_btn_new)) {
				//comm_request = 25;
				batch_cmd = [25, 15];
				batch_cnt = 0;
				batch_cnt_top = 2;
			}
		}
		p_event_btn_mod.style.color = '#000000';
		p_event_btn_mod.onclick = function() {
			if (chk_btn_active(p_event_btn_mod)) {
				//comm_request = 25;
				//batch_cmd = [25, 15];
				//batch_cnt = 0;
				//batch_cnt_top = 2;
			}
		}
		p_event_btn_pedit_edit.style.color = '#000000';
		p_event_btn_pedit_edit.onclick = function() {
			if (chk_btn_active(p_event_btn_pedit_edit)) {
				if (!do_pedit) {
					do_pedit = true;
				} else {
					do_pedit = false;
					// -- write event pilots to tracker
					batch_cmd = [26, 20];
					batch_cnt = 0;
					batch_cnt_top = 2;
				}
				event_enable_pilot_edit();
			}
		}
		p_event_btn_pedit_toleft.style.color = '#000000';
		p_event_btn_pedit_toleft.onclick = function() {
			if (chk_btn_active(p_event_btn_pedit_toleft)) {
				if (do_pedit) {
					//window.alert("Pilot to left!");
					pilots_event_nr[pilots_event_count] = pilot_selected;
					pilots_event_count++;
					draw_tab_event();
				}
			}
		}
		p_event_btn_pedit_pup.style.color = '#000000';
		p_event_btn_pedit_pup.onclick = function() {
			if (chk_btn_active(p_event_btn_pedit_pup)) {
				if ((do_pedit) && (pilot_event_selected != 0xffff) && (pilot_event_selected > 0)) {
					//window.alert("Pilot to left! '" + pilot_event_selected + "'");
					var eptmp = pilots_event_nr[pilot_event_selected];
					pilots_event_nr[pilot_event_selected] = pilots_event_nr[pilot_event_selected-1];
					pilots_event_nr[pilot_event_selected-1] = eptmp;
					event_table_pilots_event_select(pilot_event_selected-1);
					draw_tab_event();
				}
			}
		}
		p_event_btn_pedit_pdown.style.color = '#000000';
		p_event_btn_pedit_pdown.onclick = function() {
			if (chk_btn_active(p_event_btn_pedit_pdown)) {
				if ((do_pedit) && (pilot_event_selected != 0xffff) && (pilot_event_selected < (pilots_event_count-1))) {
					//window.alert("Pilot to left! '" + pilot_event_selected + "'");
					var eptmp = pilots_event_nr[pilot_event_selected];
					pilots_event_nr[pilot_event_selected] = pilots_event_nr[pilot_event_selected+1];
					pilots_event_nr[pilot_event_selected+1] = eptmp;
					event_table_pilots_event_select(pilot_event_selected+1);
					draw_tab_event();
				}
			}
		}
		p_event_btn_pedit_toright.style.color = '#000000';
		p_event_btn_pedit_toright.onclick = function() {
			if (chk_btn_active(p_event_btn_pedit_toright)) {
				if ((do_pedit) && (pilot_event_selected != 0xffff)) {
					//window.alert("Pilot to left! '" + pilot_event_selected + "'");
					pilots_event_count--;
					for (var i = 0; i < pilots_event_count; i++) {
						if (i >= pilot_event_selected) {
							pilots_event_nr[i] = pilots_event_nr[i+1];
						}
					}
					draw_tab_event();
				}
			}
		}
		
		// --- event list selector
		document.getElementById("t_events0").onclick = function() { events_list_selected = 0; }
		document.getElementById("t_events1").onclick = function() { events_list_selected = 1; }
		document.getElementById("t_events2").onclick = function() { events_list_selected = 2; }
		document.getElementById("t_events3").onclick = function() { events_list_selected = 3; }
	
		// --- add results table elements
		var t_div = document.getElementById("div_event_results_table");
		// -- table body, for data
		var t_table = document.createElement("table");
		t_table.setAttribute("id", "table_event_results");
		t_table.setAttribute("class", "cls_event_res_table_table");
		t_div.appendChild(t_table);
		
		// --- add all pilots table elements
		var t_div = document.getElementById("canvas_pilots_all");
		// -- table body, for data
		var t_table = document.createElement("table");
		t_table.setAttribute("id", "table_event_pilots_all");
		//t_table.setAttribute("class", "cls_event_res_table_table");
		t_table.style.borderCollapse = "collapse";
		t_table.setAttribute("width", "100%");
		t_div.appendChild(t_table);
		
		// --- add event pilots table elements
		var t_div = document.getElementById("canvas_pilots_event");
		// -- table body, for data
		var t_table = document.createElement("table");
		t_table.setAttribute("id", "table_event_pilots_event");
		t_table.setAttribute("class", "cls_event_res_table_table");
		t_table.setAttribute("width", "100%");
		t_div.appendChild(t_table);
	}
	
	// ****** event result radio buttons clicked
	function event_result_radio_click(etmp) {
		event_result_show_type = etmp;
		draw_tab_event();
	}
	
	// ****** redraw tables n stuff on event tab
	function draw_tab_event() {
		// --- events table
		for (var p = 0; p < 4; p++) {
			// --- table column 0
			document.getElementById("t_events" + p).textContent = events_names[p + events_list_offset];
		}
		
		// --- open/close buttons
		if (event_is_open) {
			if (session_is_open) {
				p_event_btn_open.style.background = col_btn_inactive;
				p_event_btn_new.style.background = col_btn_inactive;
				p_event_btn_close.style.background = col_btn_inactive;
				p_event_btn_mod.style.background = col_btn_inactive;
				allow_pedit = false;
			} else {
				p_event_btn_open.style.background = col_btn_inactive;
				p_event_btn_new.style.background = col_btn_inactive;
				p_event_btn_close.style.background = col_btn_active;
				p_event_btn_mod.style.background = col_btn_active;
				allow_pedit = true;
			}
		} else {
			p_event_btn_open.style.background = col_btn_active;
			p_event_btn_new.style.background = col_btn_active;
			p_event_btn_close.style.background = col_btn_inactive;
			p_event_btn_mod.style.background = col_btn_inactive;
			allow_pedit = false;
		}
		event_enable_pilot_edit();
		
		// --- event results table size
		switch(event_result_show_type) {
			// -- fastest laps
			case 0:
				event_result_cnt = event_fastest_laps_cnt;
				break;
			// -- quali results
			case 1:
				event_result_cnt = event_quali_cnt;
				break;
			// -- race results
			case 2:
				event_result_cnt = 0;
				break;
		}

		// --- work on table sizes
		handle_event_table_sizes();
		
		// --- all pilots table
		var ctmp = 0;
		for (var p = 0; p < pilots_count; p++) {
			ctmp++;
			// --- table column 0
			document.getElementById("id_event_pilots_all_table_t" + p + "-0").textContent = ctmp;
			// --- table column 1
			document.getElementById("id_event_pilots_all_table_t" + p + "-1").textContent = pilots_names[p];
		}
		
		// --- event pilots table
		ctmp = 0;
		for (var p = 0; p < pilots_event_count; p++) {
			ctmp++;
			// --- table column 0
			document.getElementById("id_event_pilots_event_table_t" + p + "-0").textContent = ctmp;
			// --- table column 1
			document.getElementById("id_event_pilots_event_table_t" + p + "-1").textContent = pilots_names[pilots_event_nr[p]];
		}
		
		// --- event results table
		ctmp = 0;
		switch(event_result_show_type) {
			// -- fastest laps
			case 0:
				for (var p = 0; p < event_fastest_laps_cnt; p++) {
					ctmp++;
					// --- table column 0
					document.getElementById("id_event_res_table_t" + p + "-0").textContent = ctmp;
					// --- table column 1
					document.getElementById("id_event_res_table_t" + p + "-1").textContent = pilots_names[event_fastest_laps_pilotnr[p]];
					// --- table column 2
					document.getElementById("id_event_res_table_t" + p + "-2").textContent = "";
					// --- table column 3
					document.getElementById("id_event_res_table_t" + p + "-3").textContent = laptime2string(event_fastest_laps[p]);
				}
				break;
			// -- quali results
			case 1:
				for (var p = 0; p < event_quali_cnt; p++) {
					ctmp++;
					// --- table column 0
					document.getElementById("id_event_res_table_t" + p + "-0").textContent = ctmp;
					// --- table column 1
					document.getElementById("id_event_res_table_t" + p + "-1").textContent = pilots_names[event_quali_pilotnr[p]];
					// --- table column 2
					document.getElementById("id_event_res_table_t" + p + "-2").textContent = event_quali_laps[p];
					// --- table column 3
					document.getElementById("id_event_res_table_t" + p + "-3").textContent = laptime2string(event_quali_time[p]);
				}
				break;
		}
	}
	
	// ****** enable pilot edit
	function event_enable_pilot_edit() {
		if (allow_pedit) {
			p_event_btn_pedit_edit.style.background = col_btn_active;
			if (do_pedit) {
				p_event_btn_pedit_toleft.style.background  = col_btn_active;
				p_event_btn_pedit_pup.style.background     = col_btn_active;
				p_event_btn_pedit_pdown.style.background   = col_btn_active;
				p_event_btn_pedit_toright.style.background = col_btn_active;
			} else {
				p_event_btn_pedit_toleft.style.background  = col_btn_inactive;
				p_event_btn_pedit_pup.style.background     = col_btn_inactive;
				p_event_btn_pedit_pdown.style.background   = col_btn_inactive;
				p_event_btn_pedit_toright.style.background = col_btn_inactive;
			}
		} else {
			p_event_btn_pedit_edit.style.background    = col_btn_inactive;
			p_event_btn_pedit_toleft.style.background  = col_btn_inactive;
			p_event_btn_pedit_pup.style.background     = col_btn_inactive;
			p_event_btn_pedit_pdown.style.background   = col_btn_inactive;
			p_event_btn_pedit_toright.style.background = col_btn_inactive;
		}
	}
	
	// ****** select pilot on all pilots list
	function event_table_pilots_all_select(nr) {
		var inr = parseInt(nr);
		//window.alert(pilot_selected);
		if (do_pedit) {
			document.getElementById("id_event_pilots_all_table_t" + inr + "-" + 0).style.backgroundColor = '#666666';
			document.getElementById("id_event_pilots_all_table_t" + inr + "-" + 1).style.backgroundColor = '#666666';
			if ((pilot_selected != 0xffff) && (pilot_selected != inr)) {
				document.getElementById("id_event_pilots_all_table_t" + pilot_selected + "-" + 0).style.backgroundColor = '#000000';
				document.getElementById("id_event_pilots_all_table_t" + pilot_selected + "-" + 1).style.backgroundColor = '#000000';
			}
			pilot_selected = inr;
		}
	}
	
	// ****** select pilot on event pilots list
	function event_table_pilots_event_select(nr) {
		var inr = parseInt(nr);
		//window.alert("Pilot Event Select '" + inr + "'");
		if (do_pedit) {
			document.getElementById("id_event_pilots_event_table_t" + inr + "-" + 0).style.backgroundColor = '#666666';
			document.getElementById("id_event_pilots_event_table_t" + inr + "-" + 1).style.backgroundColor = '#666666';
			if ((pilot_event_selected != 0xffff) && (pilot_event_selected != inr)) {
				document.getElementById("id_event_pilots_event_table_t" + pilot_event_selected + "-" + 0).style.backgroundColor = '#000000';
				document.getElementById("id_event_pilots_event_table_t" + pilot_event_selected + "-" + 1).style.backgroundColor = '#000000';
			}
			pilot_event_selected = inr;
		}
	}
	
	// ****** add row to table of all pilots
	function event_table_pilots_all_add_row() {
		var t_table = document.getElementById("table_event_pilots_all");
		var t_tr = document.createElement("tr");
		t_table.appendChild(t_tr);
		for (var m = 0; m < 2; m++) {
			var t_td = document.createElement("td");
			t_td.setAttribute("id", "id_event_pilots_all_table_t" + event_table_pilots_all_size + "-" + m);
			t_td.setAttribute("class", "cls_event_table_tdr");
			t_td.setAttribute("data-indexnr", event_table_pilots_all_size);
			t_td.onclick = function() { event_table_pilots_all_select(this.dataset.indexnr); }
			t_tr.appendChild(t_td);
		}
		event_table_pilots_all_size++;
	}
	
	// ****** remove row from table of all pilots
	function event_table_pilots_all_remove_row() {
		var t_table = document.getElementById("table_event_pilots_all");
		t_table.deleteRow(t_table.length);
		event_table_pilots_all_size--;
	}
	
	// ****** add row to table of event pilots
	function event_table_pilots_event_add_row() {
		var t_table = document.getElementById("table_event_pilots_event");
		var t_tr = document.createElement("tr");
		t_table.appendChild(t_tr);
		for (var m = 0; m < 2; m++) {
			var t_td = document.createElement("td");
			t_td.setAttribute("id", "id_event_pilots_event_table_t" + event_table_pilots_event_size + "-" + m);
			t_td.setAttribute("class", "cls_event_table_tdr");
			t_td.setAttribute("data-indexnr", event_table_pilots_event_size);
			t_td.onclick = function() { event_table_pilots_event_select(this.dataset.indexnr); }
			t_tr.appendChild(t_td);
		}
		event_table_pilots_event_size++;
	}
	
	// ****** remove row from table of event pilots
	function event_table_pilots_event_remove_row() {
		var t_table = document.getElementById("table_event_pilots_event");
		t_table.deleteRow(-1);
		event_table_pilots_event_size--;
	}
	
	// ****** add row to event results table
	function event_table_results_add_row() {
		var t_table = document.getElementById("table_event_results");
		var t_tr = document.createElement("tr");
		t_table.appendChild(t_tr);
		for (var m = 0; m < 4; m++) {
			var t_td = document.createElement("td");
			t_td.setAttribute("id", "id_event_res_table_t" + event_result_table_size + "-" + m);
			t_td.setAttribute("class", "cls_event_table_tdr");
			t_tr.appendChild(t_td);
		}
		event_result_table_size++;
	}
	
	// ****** remove row from event results table
	function event_table_results_remove_row() {
		var t_table = document.getElementById("table_event_results");
		t_table.deleteRow(-1);
		event_result_table_size--;
	}
	
	// ****** handle sizes of table on event tab
	function handle_event_table_sizes() {
		// --- all pilots table
		// -- remove some rows from table?
		while (event_table_pilots_all_size > pilots_count) event_table_pilots_all_remove_row();
		// -- add some rows to table?
		while (event_table_pilots_all_size < pilots_count) event_table_pilots_all_add_row();
		
		// --- event pilots table
		// -- remove some rows from table?
		while (event_table_pilots_event_size > pilots_event_count) event_table_pilots_event_remove_row();
		// -- add some rows to table?
		while (event_table_pilots_event_size < pilots_event_count) event_table_pilots_event_add_row();
		
		// --- event results table
		// -- remove some rows from table?
		while (event_result_table_size > event_result_cnt) event_table_results_remove_row();
		// -- add some rows to table?
		while (event_result_table_size < event_result_cnt) event_table_results_add_row();
	}
	
	// ****** clear event results
	function clear_event_results() {
		event_fastest_laps_cnt = 0;
		for (var i = 0; i < event_fastest_laps.length; i++) {
			event_fastest_laps[i] = 0xffff;
		}
		for (var i = 0; i < event_fastest_laps_pilotnr.length; i++) {
			event_fastest_laps_pilotnr[i] = 0xffff;
		}
		event_quali_cnt = 0;
		for (var i = 0; i < event_quali_laps.length; i++) {
			event_quali_laps[i] = 0;
		}
		for (var i = 0; i < event_quali_time.length; i++) {
			event_quali_time[i] = 0xffff;
		}
		for (var i = 0; i < event_quali_pilotnr.length; i++) {
			event_quali_pilotnr[i] = 0xffff;
		}
	}
	