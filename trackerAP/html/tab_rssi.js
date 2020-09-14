	// -------------------------------------------------------------------------------------------------------------------------
	// rssi graph tab
	// -------------------------------------------------------------------------------------------------------------------------
	// ****** translate rssi graphs
	function translate_rssi(dx, save_pos) {
		if (g_len < g_width) {
			dx = 0;
		} else if (dx < -(g_len - g_width)) {
			dx = -(g_len - g_width);
		}
		if (save_pos) g_pos = dx;
		p_canvas_rssi.style.transform = "translateX(" + dx + "px)";
	}
	
	// ****** draw rssi graphs
	function draw_rssi() {
		// --- walk through channels
		var box = document.querySelector('div');
		var sizex = box.clientWidth - 20;
		// -- graph size
		document.getElementById("div_rssi_top").style.width = sizex + "px";
		//g_width = sizex - 8;
		g_width = sizex - 2;
		document.getElementById("div_rssi_graph").style.width = g_width + "px";
		// -- handle canvas
		let canvas = document.getElementById("canvas_rssi");
		//canvas.height = 134;
		canvas.height = 134*4 + 80;
		w_tmp = tstamp_rssi;
		if (g_width > tstamp_rssi) w_tmp = g_width;
		canvas.width = w_tmp;
		g_len = tstamp_rssi;
		ybase = 0;
		if (canvas && canvas.getContext) {
			for (var i = 0; i < 4; i++) {
				var ctx = canvas.getContext('2d');
				// -- mark exceptions
				for (var k = 0; k < ex_count[i]; k++) {
					ctx.beginPath();
					ctx.strokeStyle = "#990066";
					ctx.fillStyle = "#440022";
					var ex_x = (exceptions[i][k] & 0x00ffffff)/100
					if (exceptions[i][k] & 0x01000000) {
						ctx.fillRect(ex_x-60, ybase+1, 120, 133);	// 12s
					} else {
						ctx.fillRect(ex_x-30, ybase+1, 60, 133);	// 6s
					}
					ctx.stroke();
				}
				// -- mark new exception position
				if ((i == lt_chn) && (lt_show_tmp)) {
					ctx.beginPath();
					ctx.strokeStyle = "#ff00aa";
					ctx.moveTo(lt_clickposX, ybase + 133);
					ctx.lineTo(lt_clickposX, ybase + 1);
					ctx.stroke();
				}
				// -- mark hits
				for (var k = 0; k < hit_count[i]; k++) {
					ctx.beginPath();
					ctx.strokeStyle = "#ffaa00";
					ctx.moveTo(hits[i][k], ybase + 133);
					ctx.lineTo(hits[i][k], ybase + 1);
					ctx.stroke();
				}
				// -- mark detection level
				ctx.beginPath();
				ctx.moveTo(1, ybase + 133-(det_level_mod[i]>>5));
				ctx.lineTo(tstamp_rssi-1, ybase + 133-(det_level_mod[i]>>5));
				ctx.strokeStyle = "#ffaa00";
				ctx.stroke();
				// -- draw graph
				ctx.beginPath();
				ctx.strokeStyle = "#66ff66";
				ctx.moveTo(1, ybase + 133-(rssi[i][0]>>5));
				for (var k=0; k<tstamp_rssi;k++) {
					ctx.lineTo(k, ybase + 133-(rssi[i][k]>>5));
				}
				ctx.stroke();
				// -- draw frame around channel diagram
				ctx.beginPath();
				ctx.strokeStyle = "#00bb00";
				ctx.rect(0, ybase, w_tmp, 134);
				ctx.stroke();
				// - move downward for drawing
				ybase += 156;
			}
		}
		
		//canvas.style.display = "none";
	}
	
	// ****** load rssi data from rt
	function load_rssi() {
		comm_request = 5;
	}
	
		// ****** handle rssi exception menu
	function handle_rssiex(show) {
		// --- show menu?
		if (show) {
			document.getElementById("div_rssi_exception").style.display = "block";
		} else {
			document.getElementById("div_rssi_exception").style.display = "none";
			// -- reset temp marker
			lt_show_tmp = false;
		}
	}
	
	// ****** add exception
	function add_exception(tchn, tpos, ttype) {
		// --- write new exception to array
		switch(ttype) {
			// -- 6s
			case 0:
				exceptions[tchn][ex_count[tchn]] = tpos * 100;
				ex_count[tchn]++;
				break;
			// -- 12s
			case 1:
				exceptions[tchn][ex_count[tchn]] = (tpos * 100) + 0x01000000;
				ex_count[tchn]++;
				break;
			// -- delete exception
			case 2:
				break;
		}
		// --- redraw rssi graph
		draw_rssi();
		// --- hide menu
		handle_rssiex(false);
		// -- send exceptions to tracker
		comm_request = 7;
	}
	
	// ****** delete exception
	function delete_exception(tchn, tpos) {
		// --- delete new exception from array
		var itype = 0;
		var ipos = 0;
		var idelta = 0;
		var found = false;
		for (var i = 0; i < ex_count[tchn]; i++) {
			// -- handle by type
			itype = exceptions[tchn][i] & 0x0f000000;
			ipos = (exceptions[tchn][i] & 0x00ffffff) / 100;
			switch(itype) {
				// -- 6s
				case 0x00000000:
					idelta = 30;
					break;
				// -- 12s
				case 0x01000000:
					idelta = 60;
					break;
			}
			// -- do compare
			if ((tpos > (ipos-idelta)) && (tpos < (ipos+idelta)) && (!found)) {
				found = true;
				ex_count[tchn]--;
			}
			// -- if found, copy exception
			if (found) {
				exceptions[tchn][i] = exceptions[tchn][i+1];
			}
		}
		// --- redraw rssi graph
		draw_rssi();
		// --- hide menu
		handle_rssiex(false);
		// -- send exceptions to tracker
		comm_request = 7;
	}
	
	// -------------------------------------------------------------------------------------------------------------------------
	// rssi tab mouse/touch handling
	// -------------------------------------------------------------------------------------------------------------------------
	// ****** onMouseDown event, scroll rssi graph
	function onMouseDown(e) {
		// --- exit/hide exception menu
		handle_rssiex(false);
		g_startx = e.clientX;
		g_starty = e.clientY;
		if (e.button === 2) {
			//alert("From JS: right button is down!")
			// - get x offset
			lt_clickposX = e.clientX - document.getElementById('div_rssi_top').offsetLeft - g_pos;
			tmp_yoffset = document.getElementById('div_header_l').offsetHeight;
			lt_chn = Math.floor((g_starty-tmp_yoffset) / 156);
			lt_show_tmp = true;
			handle_rssiex(true);
		} else {
			// -- determine chn
			tmp_yoffset = document.getElementById('div_header_l').offsetHeight;
			g_chn = 0;
			g_chn = Math.floor((g_starty-tmp_yoffset) / 156);
			g_drag = true;
			g_touch = false;
			// --- translate rssi graph OR change detection level
			if (g_startx > 70) 	g_level = false;
			else				g_level = true;
		}
	}
	function onTouchDown(e) {
		// --- start timer for long touch
		lt_timer = setTimeout(onlongtouch, lt_touchduration);
		// --- exit/hide exception menu
		var touch = e.touches[0];
		g_startx = Math.floor(touch.clientX);
		g_starty = Math.floor(touch.clientY);
		//handle_rssiex(false);
		// -- determine chn
		tmp_yoffset = document.getElementById('div_header_l').offsetHeight;
		g_chn = 0;
		g_chn = Math.floor((g_starty-tmp_yoffset) / 156);
		g_drag = true;
		g_touch = true;
		// --- translate rssi graph OR change detection level
		if (g_startx > 100) 	g_level = false;
		else				g_level = true;
		//g_drag = true;
		//g_touch = true;
	}
	
	// ****** onMouseMove event, scroll rssi graph
	function onMouseMove(e) {
		// --- clear long touch timer, if moved too much...
		if (lt_timer) {
			var touch = e.touches[0];
			var tdx = Math.floor(g_startx - touch.clientX);
			var tdy = Math.floor(g_starty - touch.clientY);
			if ((Math.abs(tdx) > 2) || (Math.abs(tdy) > 2)) {
				clearTimeout(lt_timer);
			}
		}
		// --- do move'n stuff
		if (g_drag === true) {
			if (g_level == false) {
				// --- translate rssi graph
				if (!g_touch) {
					g_diffx = g_startx - e.clientX;
				} else {
					var touch = e.touches[0];
					g_diffx = g_startx - Math.floor(touch.clientX);
				}
				if (g_len < g_width) {
					g_pos_drag = 0;
				} else {
					g_pos_drag = g_pos - g_diffx;
					if (g_pos_drag <= 0) {
						if (g_pos_drag < -(g_len - g_width)) {
							g_pos_drag = -(g_len - g_width);
							g_auto = true;
						} else {
							g_auto = false;
						}
					} else {
						g_pos_drag = 0;
					}
				}
				//g_auto = false;
				translate_rssi(g_pos_drag, false);
			} else {
				// --- modify detection level
				//det_level_auto = false;
				if (!g_touch) {
					g_diffy = g_starty - e.clientY;
				} else {
					var touch = e.touches[0];
					g_diffy = g_starty - Math.floor(touch.clientY);
				}
				g_diffy = Math.floor(g_diffy * 12);
				det_level_mod[g_chn] = det_level[g_chn] + g_diffy;
				if (det_level_mod[g_chn] < 0x100) det_level_mod[g_chn] = 0x100;
				if (det_level_mod[g_chn] > 0xfff) det_level_mod[g_chn] = 0xfff;
				draw_rssi();
			}
		}
	}
	
	// ****** looong touch event, exception stuff
	function do_longtouch() {
		// - get x offset
		lt_clickposX = g_startx - document.getElementById('div_rssi_top').offsetLeft - g_pos;
		tmp_yoffset = document.getElementById('div_header_l').offsetHeight;
		lt_chn = Math.floor((g_starty-tmp_yoffset) / 156);
		lt_show_tmp = true;
		handle_rssiex(true);
	}
	
	// ****** onMouseUp event, scroll rssi graph
	function onMouseUp(e) {
		// --- clear long touch timer, if moved too much...
		if (lt_timer) {
			clearTimeout(lt_timer);
		}
		if (g_level == false) {
			g_pos = g_pos_drag;
		} else {
			det_level[g_chn] = det_level_mod[g_chn];
			// -- send changed levels to tracker
			comm_request = 6;
		}
		g_drag = false;
	}
	
	// ****** onMouseLeave event (pointer left the playing field), scroll rssi graph
	function onMouseLeave(e) {
		if (g_level == false) {
			g_pos = g_pos_drag;
		} else {
			det_level[g_chn] = det_level_mod[g_chn];
			// -- send changed levels to tracker
			comm_request = 6;
		}
		g_drag = false;
	}
	
	// ****** add event
	function addEvent(name, el, func) {
		if (el.addEventListener) {
			//el.addEventListener(name, func, false);
			el.addEventListener(name, func, true);
		} else if (el.attachEvent) {
			el.attachEvent('on' + name, func);
		} else {
			el[name] = func;
		}
	}
