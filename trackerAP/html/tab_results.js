	// -------------------------------------------------------------------------------------------------------------------------
	// results tab
	// -------------------------------------------------------------------------------------------------------------------------
	
	// ****** handle tablesize
	function init_tab_results() {
		// --- create rows in result table
        for (var r = 0; r < totalRows; r++) table_AddRow(r);
		// --- table properties
		// -- top, event/heat etc
		document.getElementById("tt0-0").style.width = "48vw";
		document.getElementById("tt0-3").style.width = "48vw";
		// -- top, top4s, left
		document.getElementById("ttl0-0").style.width = "4vw";
		document.getElementById("ttl0-1").style.width = "26vw";
		document.getElementById("ttl0-2").style.width = "16vw";
		// -- top, top4s, right
		document.getElementById("ttr0-0").style.width = "4vw";
		document.getElementById("ttr0-1").style.width = "22vw";
		document.getElementById("ttr0-2").style.width = "4vw";
		document.getElementById("ttr0-3").style.width = "16vw";
		// -- results, pilot names
		document.getElementById("trn2-0").style.width = "24vw";
		document.getElementById("trn2-1").style.width = "26vw";
		document.getElementById("trn2-2").style.width = "26vw";
		document.getElementById("trn2-3").style.width = "26vw";
		// -- results
		document.getElementById("tres0-0").style.width = "4vw";
		document.getElementById("tres0-1").style.width = "20vw";
		document.getElementById("tres0-2").style.width = "6vw";
		document.getElementById("tres0-3").style.width = "20vw";
		document.getElementById("tres0-4").style.width = "6vw";
		document.getElementById("tres0-5").style.width = "20vw";
		document.getElementById("tres0-6").style.width = "6vw";
		document.getElementById("tres0-7").style.width = "20vw";
	}
	
	// ****** handle tablesize
	function table_Size(tgtsize) {
		// --- table is smaller than target size
		if (totalRows < tgtsize) {
			var addnr = tgtsize - totalRows;
			for (var i = 0; i < addnr; i++) {
				table_AddRow(i + totalRows);
			}
			totalRows = tgtsize;
		}
		// --- table is bigger than target size
		if (totalRows > tgtsize) {
			var subnr = totalRows - tgtsize;
			for (var i = 0; i < subnr; i++) {
				table_RemoveRow(totalRows - i - 1);
			}
			totalRows = tgtsize;
		}
	}
	
	// ****** add row to table
	function table_AddRow(i) {
		var row = document.createElement("tr");
		var tbl = document.getElementById("table_heat_res");
		// -- create and append cells to row
		for (var c = 0; c < cellsInRow; c++) {
			var cell = document.createElement("td");
			cell.setAttribute("id", "tres" + (i) + "-" + c);
			cell.style.borderSpacing  = "0";
			cell.style.padding = "0px 0px 0px 0px";
			cell.style.border = "1px solid #004000";
			var cellText = document.createTextNode(cell.id);
			cell.appendChild(cellText);
			row.appendChild(cell);
		}
		// -- add the row to the end of the table body
		tbl.appendChild(row);
	}
	
	// ****** remove row from table
	function table_RemoveRow(i) {
		document.getElementById("table_heat_res").deleteRow(i);
	}
	
	// ****** print laps to table
	function printLaps() {
		// --- fill top info table
		//document.getElementById("tt0-0").textContent = session_name + " " + heat_nr_current + " / " + sessions_heats_count;
		document.getElementById("tt0-0").textContent = session_name;
		document.getElementById("tt0-3").textContent = event_name;
		document.getElementById("tt1-0").textContent = heat_name;
		
		// --- fill fastest laps
		for (var i = 0; i < 4; i++) {
			if (pos_fastest_lapnr[i] != 0xff) {
				document.getElementById("ttl" + i + "-1").textContent = pilots_names[heat_pilots[pos_fastest[i]]];
				document.getElementById("ttl" + i + "-2").textContent = laptime2string(laps[pos_fastest[i]][pos_fastest_lapnr[i]]/1000);
			} else {
				document.getElementById("ttl" + i + "-1").textContent = " ";
				document.getElementById("ttl" + i + "-2").textContent = " ";
			}
		}
		
		// --- fill position table
		for (var i = 0; i < 4; i++) {
			if (pos_heat_laps[i] != 0) {
				document.getElementById("ttr" + i + "-1").textContent = pilots_names[heat_pilots[pos_heat[i]]];
				document.getElementById("ttr" + i + "-2").textContent = pos_heat_laps[i];
				document.getElementById("ttr" + i + "-3").textContent = laptime2string(pos_heat_times[i]/1000);
			} else {
				document.getElementById("ttr" + i + "-1").textContent = " ";
				document.getElementById("ttr" + i + "-2").textContent = " ";
				document.getElementById("ttr" + i + "-3").textContent = " ";
			}
		}
		
		// --- print pilot names to table top
		document.getElementById("trn3-0").textContent = pilots_names[heat_pilots[0]];
		document.getElementById("trn3-1").textContent = pilots_names[heat_pilots[1]];
		document.getElementById("trn3-2").textContent = pilots_names[heat_pilots[2]];
		document.getElementById("trn3-3").textContent = pilots_names[heat_pilots[3]];
		// --- fill laptime table
		var tltmp = 1;
		for (var i = 0; i < 4; i++) {
			if (tltmp < lap_count[i]) tltmp = lap_count[i];
		}
		var row_glob = 0;
		var lines = 0;
		for (var i = 0; i < 4; i++) {
			table_Size(tltmp);
			for (var k = 0; k < tltmp; k++) {
				var dtmp = laps[i][k] / 1000;
				if (dtmp) {
					document.getElementById("tres" + k + "-" + (i*2)).textContent = k+1;
					document.getElementById("tres" + k + "-" + (i*2+1)).textContent = laptime2string(dtmp);
				} else {
					if (k != 0) {
						document.getElementById("tres" + k + "-" + (i*2)).textContent = "";
						document.getElementById("tres" + k + "-" + (i*2+1)).textContent = "";
					} else {
						document.getElementById("tres" + k + "-" + (i*2)).textContent = "1";
						document.getElementById("tres" + k + "-" + (i*2+1)).textContent = "---";
					}
				}
			}
		}
		// --- fill status bar
		var sTmp = "";
		var tstamp = timestamp / 10;
		var min = Math.floor(tstamp / 60);
		tstamp = tstamp - (min * 60);
		if (tstamp > 10) {
			sTmp = min + ":" + tstamp.toFixed(1);
		} else {
			sTmp = min + ":0" + tstamp.toFixed(1);
		}
		document.getElementById("div_header_r").textContent = sTmp;
		// --- handle clear/commit
		var bt_c = document.getElementById("btn_clear");
		if (bt_c != null) {
			if (use_event_mode != 0) {
				bt_c.textContent = "Commit";
			} else {
				bt_c.textContent = "Clear";
			}
		}
	}
