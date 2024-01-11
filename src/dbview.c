/*===--------------------------------------------------------------------------------------------===
 * dbview.c
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

typedef struct {
	int	id;
	veh_t	*veh;
	bool	sel;
} rec_t;

typedef struct {
	db_t	*db;
	int	offset;
	int	sel;
	
	rec_t	*veh;
	int	num_veh;
} dbview_t;

static void
update_veh(dbview_t *view) {
	size_t num_veh = stock_db_get_count(view->db);
	
	view->veh = safe_realloc(view->veh, num_veh * sizeof(rec_t));
	view->num_veh = (int)num_veh;
	
	int *veh_ids = safe_calloc(view->num_veh, sizeof(int));
	stock_db_get_list(view->db, veh_ids, num_veh);
	
	for(int i = 0; i < view->num_veh; ++i) {
		view->veh[i].id = veh_ids[i];
		view->veh[i].veh = stock_db_get(view->db, veh_ids[i]);
		view->veh[i].sel = false;
	}
	free(veh_ids);
}

static const char *type_name[] = {
	[VEH_TYPE_UNKNOWN] = "unknown",
	[VEH_TYPE_LOK] = "locomotive",
	[VEH_TYPE_VAN] = "van",
	[VEH_TYPE_COACH] = "coach",
	[VEH_TYPE_WAGON] = "wagon",
	[VEH_TYPE_CONTROL] = "control",
	[VEH_TYPE_RAILCAR] = "railcar",
};

static void
dbview_draw_list(dbview_t *view) {
	
#define ID_WIDTH	(6)
#define CLASS_WIDTH	(12)
#define TYPE_WIDTH	(12)
#define SELECT_WIDTH	(2)

	int w, h;
	hexes_get_size(&w, &h);
	int desc_width = w - (13 + ID_WIDTH + CLASS_WIDTH + TYPE_WIDTH + SELECT_WIDTH);
	
	for(int i = 0; i < h-2; ++i) {
		hexes_cursor_go(0, i+1);
		if(i == view->sel)
			term_reverse(stdout);
		
		int idx = i + view->offset;
		if(i < 0 || i >= view->num_veh) {
			ui_line("| %c%-*s | %-*s | %-*s | %-*s%c |",
				' ',
				CLASS_WIDTH, "",
				ID_WIDTH, "",
				TYPE_WIDTH, "",
				desc_width, "",
				' ');
		} else {
			rec_t *rec = &view->veh[idx];
			veh_t *veh = rec->veh;
			if(!veh) continue;
			ui_line("|%c %-*.*s | %*d | %-*s | %-*.*s %c|",
				rec->sel ? '*' : ' ',
				CLASS_WIDTH, CLASS_WIDTH, veh->class,
				ID_WIDTH, veh->num,
				TYPE_WIDTH, type_name[veh->type],
				desc_width, desc_width, veh->desc,
				rec->sel ? '*' : ' ');
		}
		term_style_reset(stdout);
	}
}

static void
dbview_draw(dbview_t *view) {
	hexes_clear_screen();
	ui_title(" Rolling Stock Database - Vehicles");
	dbview_draw_list(view);
	ui_prompt(" [Q]uit    [A]dd    [E]dit    [D]elete    [S]elect for s[H]unting");
}

static void
shunting_puzzle(dbview_t *view) {
	int num_veh = 0;
	for(int i = 0; i < view->num_veh; ++i) {
		if(view->veh[i].sel)
			num_veh += 1;
	}
	
	if(!num_veh)
		return;
	
	const veh_t **stock = safe_calloc(num_veh, sizeof(veh_t *));
	
	for(int i = 0, j = 0; i < view->num_veh; ++i) {
		if(view->veh[i].sel)
			stock[j++] = view->veh[i].veh;
	}
	
	show_shuntview(view->db, stock, num_veh);
	free(stock);
}


static bool
dbview_update(dbview_t *view) {
	UNUSED(view);
	
	int c = hexes_get_key_raw();
	
	veh_t *veh = NULL;
	rec_t *rec = NULL;
	if(view->sel >= 0 && view->sel < view->num_veh) {
		rec = &view->veh[view->sel];
		veh = rec->veh;
	}
	
	switch(c) {
	case KEY_CTRL_C:
	case KEY_CTRL_D:
	case KEY_CTRL_Q:
	case 'q':
	case 'Q':
		return false;
	case 'e':
	case 'E':
		if(veh) {
			show_addview(view->db, veh);
			update_veh(view);
			view->sel = 0;
		}
		break;
	case 'd':
	case 'D':
	case KEY_BACKSPACE:
		if(veh) {
			stock_db_delete(view->db, veh);
			update_veh(view);
			view->sel = 0;
		}
		break;
	case 'a':
	case 'A':
		show_addview(view->db, NULL);
		update_veh(view);
		view->sel = 0;
		break;
	case 's':
	case 'S':
		if(rec) {
			rec->sel = !rec->sel;
		}
		break;
		
		case 'h':
		case 'H':
		shunting_puzzle(view);
		break;
		
	case KEY_ARROW_DOWN:
		view->sel = MIN(view->num_veh-1, view->sel+1);
		break;
	case KEY_ARROW_UP:
		view->sel = MAX(0, view->sel-1);
		break;
	default:
		break;
	}
	return true;
}

void show_dbview(db_t *db) {
	dbview_t view = {
		.db = db,
		.offset = 0,
		.sel = 0,
	};
	update_veh(&view);
	do {
		dbview_draw(&view);
	} while(dbview_update(&view));
	if(view.veh)
		free(view.veh);
}

