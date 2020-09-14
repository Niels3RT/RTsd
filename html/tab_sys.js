	// -------------------------------------------------------------------------------------------------------------------------
	// system menu tab
	// -------------------------------------------------------------------------------------------------------------------------
	// ****** update values of entries in system tab
	function update_sys() {
		// --- channels
		var band = "R";
		document.getElementById("div_sys_chn0").textContent = "Channel 1: " + band + "" + (cfg_chn[0]+1) + " " + cfg_frq[0];
		document.getElementById("div_sys_chn1").textContent = "Channel 2: " + band + "" + (cfg_chn[1]+1) + " " + cfg_frq[1];
		document.getElementById("div_sys_chn2").textContent = "Channel 3: " + band + "" + (cfg_chn[2]+1) + " " + cfg_frq[2];
		document.getElementById("div_sys_chn3").textContent = "Channel 4: " + band + "" + (cfg_chn[3]+1) + " " + cfg_frq[3];
		// -- detect parameters
		document.getElementById("div_sys_det_dead").textContent		= "Detect Dead: " + cfg_dead + "s";
		document.getElementById("div_sys_det_percent").textContent	= "Detect Level: " + cfg_percent + "%";
		document.getElementById("div_sys_det_min").textContent		= "Detect Min: " + cfg_min + "%";
		document.getElementById("div_sys_det_filter").textContent	= "RSSI Filter: " + cfg_filter;
	}
	
	// ****** change value from/in edit
	function edit_change_value(type) {
		// --- get value
		var value = parseInt(document.getElementById("sys_edit").value);
		switch(type) {
			case 1:		// +
				document.getElementById("sys_edit").value = value + 1;
				break;
			case 2:		// -
				document.getElementById("sys_edit").value = value - 1;
				break;
			case 3:		// - cancel
				document.getElementById("div_sys_edit").style.display = "none";
				break;
			case 4:		// - ok
				document.getElementById("div_sys_edit").style.display = "none";
				switch(edit_target) {
					case 1:		// dead time
						cfg_dead = value;
						break;
					case 2:		// detect level (% of max)
						cfg_percent = value;
						break;
					case 3:		// detect minimum max to be considered sane
						cfg_min = value;
						break;
					case 4:		// rssi rolling window filter
						cfg_filter = value;
						break;
					default:
						return;
						break;
				}
				break;
			default:
				return;
				break;
		}
		update_sys();
	}
	
	// ****** handle edit window
	function handle_edit(show) {
		// --- save target
		edit_target = show;
		// --- fill with value
		switch(show) {
			case 1:		// dead time
				document.getElementById("sys_edit").value = cfg_dead;
				break;
			case 2:		// detect level (% of max)
				document.getElementById("sys_edit").value = cfg_percent;
				break;
			case 3:		// detect minimum max to be considered sane
				document.getElementById("sys_edit").value = cfg_min;
				break;
			case 4:		// rssi rolling window filter
				document.getElementById("sys_edit").value = cfg_filter;
				break;
			default:
				return;
				break;
		}
		// --- show edit window
		var edit_tmp = document.getElementById("div_sys_edit");
		edit_tmp.style.display = "block";
		
	}
	
	// ****** change value in menu
	function menu_change_value(ele) {
		var value = parseInt(ele.id.substring(ele.id.length-1));
		cfg_frq[menu_target] = f_chnR[value];
		cfg_chn[menu_target] = value;
		update_sys();
		handle_menu(false);
	}
	
	// ****** handle menu
	function handle_menu(show) {
		// --- show menu?
		if (show) {
			var ment = document.getElementsByClassName("cls_menu_entry");
			for (var i = 0; i < ment.length; i++) ment[i].style.display = "block";
		} else {
			var ment = document.getElementsByClassName("cls_menu_entry");
			for (var i = 0; i < ment.length; i++) ment[i].style.display = "none";
		}
	}
