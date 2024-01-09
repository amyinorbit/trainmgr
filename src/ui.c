/*===--------------------------------------------------------------------------------------------===
 * ui.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "ui.h"
#include <stdarg.h>

void
ui_start() {
	hexes_show_cursor(false);
	hexes_set_alternate(true);
	hexes_raw_start();
}

void
ui_end() {
	hexes_raw_stop();
	hexes_set_alternate(false);
	hexes_show_cursor(true);
}

static void
draw_line(int w, const char *fmt, va_list args) {
	char buffer[512];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	printf("%-*.*s", w, w, buffer);
}

void
ui_text(int w, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	draw_line(w, fmt, args);
	va_end(args);
}

void
ui_line(const char *fmt, ...) {
	int w;
	hexes_get_size(&w, NULL);
	
	va_list args;
	va_start(args, fmt);
	draw_line(w, fmt, args);
	va_end(args);
}

void
ui_title(const char *fmt, ...) {
	int w;
	hexes_get_size(&w, NULL);
	
	hexes_cursor_go(0, 0);
	term_set_bold(stdout, true);
	term_reverse(stdout);
	
	va_list args;
	va_start(args, fmt);
	
	draw_line(w, fmt, args);
	
	va_end(args);
	term_style_reset(stdout);
}

void
ui_prompt(const char *fmt, ...) {
	int w, h;
	hexes_get_size(&w, &h);
	
	hexes_cursor_go(0, h-1);
	term_reverse(stdout);
	
	va_list args;
	va_start(args, fmt);
	
	draw_line(w, fmt, args);
	
	va_end(args);
	term_style_reset(stdout);
}

