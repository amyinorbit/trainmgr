/*===--------------------------------------------------------------------------------------------===
 * addview.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "ui.h"
#include "views.h"
#include <utils/helpers.h>

#define FIELD_CAP	(128)
typedef struct {
	bool		numeric;
	char		txt[FIELD_CAP];
	int		cap;
	int		len;
	int 		cur;
} ui_field_t;


static void
ui_field_draw(const char *label, const ui_field_t *field, bool highlight, int label_w, int max_w) {
	int label_size = MIN(label_w, max_w);
	ui_text(label_size, "%-*.*s", label_size, label_size, label);
	
	int size = MIN(field->cap, (max_w - label_w));
	if(highlight)
		term_reverse(stdout);
	term_set_underline(stdout, true);
	ui_text(size, "%-*.*s", size, size, field->txt);
	term_style_reset(stdout);
}

static inline bool char_match(bool numeric, int c) {
	if(c >= '0' && c <= '9') return true;
	if(numeric) return false;
	return (c >= 'a' && c <= 'z')
	    || (c >= 'A' && c <= 'Z')
	    || c == ' ' || c == '/' || c == '-' || c == '(' || c == ')';
}

static bool
ui_field_input(ui_field_t *field, int c) {

	switch(c) {
	case KEY_BACKSPACE:
		if(field->len && field->cur) {
			memmove(field->txt + field->cur - 1,
				field->txt + field->cur,
				field->len - field->cur);
			field->cur = MAX(0, field->cur - 1);
			field->len = MAX(0, field->len - 1);
			field->txt[field->len] = '\0';
		}
		return true;
	case KEY_ARROW_LEFT:
		field->cur = MAX(0, field->cur - 1);
		return true;
	case KEY_ARROW_RIGHT:
		field->cur = MIN(field->len, field->cur + 1);
		return true;
		
	}
	
	if(!char_match(field->numeric, c))
		return false;
	
	if(field->len + 1 > field->cap || field->len + 2 > FIELD_CAP)
		return true;
	
	memmove(field->txt + field->cur + 1,
		field->txt + field->cur,
		field->len - field->cur);
	field->txt[field->cur] = (char)c;
	field->cur++;
	field->len++;
	field->txt[field->len] = '\0';
	
	return true;
}


typedef struct {
	db_t		*db;
	veh_t		*veh;
	ui_field_t	fields[3];
	int		sel;
} addview_t;


static void
addview_draw(addview_t *view) {
	UNUSED(view);
	
	static const char *labels[] = {
		"Class",
		"Running Num.",
		"Description"
	};
	
	int w, h;
	hexes_get_size(&w, &h);
	
	hexes_clear_screen();
	ui_title(" Rolling Stock Database - Add Vehicle");
	
	int max_label_size = 15;
	int max_field_size = 30;
	int form_w = MIN(max_label_size + max_field_size, w);
	int x = MAX(0, (w/2) - (form_w / 2));
	int y = MAX(1, (h/2) - 1);
	
	int cur_x, cur_y;
	
	for(int i = 0; i < 3; ++i) {
		const ui_field_t *f = &view->fields[i];
		if(i == view->sel) {
			cur_x = MIN(x + max_label_size + MIN(f->cur, f->cap-1), w);
			cur_y = y + i;
		}
		hexes_cursor_go(x, y + i);
		ui_field_draw(labels[i], f, i == view->sel, max_label_size, form_w);
	}
	
	// Draw the OK prompt
	// "[      OK      ]"
	if(view->sel == -1)
		term_reverse(stdout);
	x = MAX(0, (w/2) - 8);
	hexes_cursor_go(x, y + 4);
	ui_text(MIN(16, w), "[      OK      ]");
	
	
	ui_prompt(" [tab]: next field    [q]: cancel");
	
	hexes_show_cursor(view->sel != -1);
	hexes_cursor_go(cur_x, cur_y);
}

static bool
check_edit_conflict(db_t *db, veh_t *veh, int num) {
	return stock_db_get(db, num) == veh;
}

static bool
validate_veh(addview_t *view) {
	for(int i = 0; i < 3; ++i) {
		if(!view->fields[i].len)
			return false;
	}
	
	
	int num = atoi(view->fields[1].txt);
	if(!check_edit_conflict(view->db, view->veh, num))
		return false;
	
	veh_t *veh = view->veh ?
		view->veh :
		safe_calloc(1, sizeof(*veh));

	veh->num = num;
	strncpy(veh->class, view->fields[0].txt, sizeof(veh->class));
	strncpy(veh->desc, view->fields[2].txt, sizeof(veh->desc));
	
	if(view->veh)
		stock_db_update(view->db, veh);
	else
		stock_db_add(view->db, veh);
	return true;
}

static bool
addview_update(addview_t *view) {
	UNUSED(view);
	
	int c = hexes_get_key_raw();
	
	if(view->sel >= 0 && view->sel < 3) {
		ui_field_t *f = &view->fields[view->sel];
		if(ui_field_input(f, c))
			return true;
	}
	
	switch(c) {
	case KEY_CTRL_C:
	case KEY_CTRL_D:
	case KEY_CTRL_Q:
	case KEY_ESC:
	case 'q':
		return false;
	case '\t':
		view->sel = (view->sel + 1);
		if(view->sel == 3)
			view->sel = -1;
		break;
	case KEY_RETURN:
		view->sel = (view->sel + 1);
		if(view->sel == 3) {
			if(validate_veh(view))
				return true;
			view->sel = -1;
		}
		break;
	case KEY_ARROW_UP:
		view->sel = MAX(0, view->sel - 1);
		break;
	case KEY_ARROW_DOWN:
		view->sel = MIN(2, view->sel + 1);
		break;
	default:
		break;
	}
	return true;
}

static void
lift_field(ui_field_t *f, const char *src) {
	strncpy(f->txt, src, f->cap-1);
	f->txt[f->cap-1] = '\0';
	f->len = f->cur = strlen(f->txt);
}

static void
lift_field_num(ui_field_t *f, int num) {
	f->len = snprintf(f->txt, sizeof(f->txt), "%d", num);
	f->cur = f->len;
}

void
show_addview(db_t *db, veh_t *veh) {
	addview_t view = {
		.db = db,
		.veh = veh
	};
	
	view.fields[0] = (ui_field_t){
		.numeric = false,
		.cap = 15,
	};
	view.fields[1] = (ui_field_t){
		.numeric = true,
		.cap = 15,
	};
	view.fields[2] = (ui_field_t){
		.numeric = false,
		.cap = 30,
	};
	
	if(veh) {
		lift_field(&view.fields[0], veh->class);
		lift_field(&view.fields[2], veh->desc);
		lift_field_num(&view.fields[1], veh->num);
	}
	
	do {
		addview_draw(&view);
	} while(addview_update(&view));
}

