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
#include "stock.h"
#include <utils/helpers.h>

typedef struct {
	db_t	*db;
	int	offset;
	int	sel;
	
	int	*veh_ids;
	int	num_veh;
} dbview_t;

static void
update_veh_ids(dbview_t *view) {
	if(view->veh_ids && view->num_veh) {
		free(view->veh_ids);
	}
	size_t num_veh = stock_db_get_count(view->db);
	
	view->veh_ids = safe_realloc(
		view->veh_ids,
		num_veh * sizeof(int));
	view->num_veh = (int)num_veh;
	stock_db_get_list(view->db, view->veh_ids, num_veh);
}

static void
dbview_draw_list(dbview_t *view) {
	
#define ID_WIDTH	(6)
#define CLASS_WIDTH	(12)

	int w, h;
	hexes_get_size(&w, &h);
	int desc_width = w - (10 + ID_WIDTH + CLASS_WIDTH);
	
	for(int i = 0; i < h-2; ++i) {
		hexes_cursor_go(0, i+1);
		
		int idx = i + view->offset;
		if(i < 0 || i >= view->num_veh) {
			ui_line("| %-*s | %-*s | %-*s |",
				CLASS_WIDTH, "",
				ID_WIDTH, "",
				desc_width, "");
		} else {
			veh_t *veh = stock_db_get(view->db, view->veh_ids[idx]);
			if(!veh) continue;
			ui_line("| %-*.*s | %*d | %-*.*s |",
				CLASS_WIDTH, CLASS_WIDTH, veh->class,
				ID_WIDTH, veh->num,
				desc_width, desc_width, veh->desc);
		}
	}
}

static void
dbview_draw(dbview_t *view) {
	hexes_clear_screen();
	ui_title(" Rolling Stock Database 1.0");
	dbview_draw_list(view);
	ui_prompt(" [1]: Delete   [Q]: Exit");
}


static bool
dbview_update(dbview_t *view) {
	dbview_draw(view);
	
	int c = hexes_get_key_raw();
	switch(c) {
	case KEY_CTRL_C:
	case KEY_CTRL_D:
	case KEY_CTRL_Q:
	case KEY_ESC:
	case 'q':
		return false;
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
	update_veh_ids(&view);
	
	ui_start();
	while(dbview_update(&view)) {}
	ui_end();
}

