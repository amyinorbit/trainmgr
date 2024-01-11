/*===--------------------------------------------------------------------------------------------===
 * shuntview.c
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
	int		tgt_num;
	int		num_veh;
	const veh_t	**stock;
	const veh_t	**train;
} shunt_view_t;

static void
draw_box(int w, int max_w, int x, int y, const char *txt) {
	max_w = MAX(0, max_w - x);
	if(!max_w)
		return;
	w = MIN(w, max_w);
	int small_w = MIN(0, w-2);
	
	term_reverse(stdout);
	hexes_cursor_go(x, y);
	ui_text(w, "%*s", w, "");
	hexes_cursor_go(x, y+1);
	ui_text(w, " %*s ", small_w, txt);
	hexes_cursor_go(x, y+2);
	ui_text(w, "%*s", w, "");
	term_style_reset(stdout);
}

static void
shuntview_draw(const shunt_view_t *view) {
	hexes_clear_screen();
	ui_title(" Rolling Stock Database - Shunting (%d > %d)",
		view->num_veh, view->tgt_num);
		
	int w, h;
	hexes_get_size(&w, &h);
	
	size_t veh_w = 0;
	
	for(int i = 0; i < view->num_veh; ++i) {
		hexes_cursor_go(1, 10+i);
		ui_line("%s", view->stock[i]->combo_desc);
		veh_w = MAX(veh_w, strlen(view->stock[i]->combo_desc));
	}
	veh_w += 2;
	
	term_set_fg(stdout, TERM_BLUE);
	draw_box(veh_w, w, 1, 2, " <Lok ");
	for(int i = 0; i < view->tgt_num; ++i) {
		draw_box(veh_w, w, 1 + (i+1) * (veh_w+1), 2, view->train[i]->combo_desc);
	}
	
	ui_prompt(" [R]eturn    [S]huffle    [I]ncrease or [D]ecrease train length");
}

static void
shuffle_train(shunt_view_t *view) {
	// Copy rolling stock list
	memcpy(view->train, view->stock, view->num_veh * sizeof(*view->stock));
	
	// Then shuffle
	for(int i = 0; i < view->num_veh; ++i) {
		int n = view->num_veh - i;
		int j = i + rand() % n;
		
		const veh_t *temp = view->train[j];
		view->train[j] = view->train[i];
		view->train[i] = temp;
	}
}

static bool
shuntview_update(shunt_view_t *view) {
	int c = hexes_get_key_raw();
	
	switch(c) {
	case 'r':
	case 'R':
		return false;
	case 's':
	case 'S':
		shuffle_train(view);
		break;
		
	case 'i':
	case 'I':
	case KEY_ARROW_UP:
		view->tgt_num = MIN(view->num_veh, view->tgt_num + 1);
		shuffle_train(view);
		break;
		
	case 'd':
	case 'D':
	case KEY_ARROW_DOWN:
		view->tgt_num = MAX(1, view->tgt_num - 1);
		shuffle_train(view);
		break;
	}
	return true;
}

void
show_shuntview(db_t *db, const veh_t **veh, int count) {
	UNUSED(db);
	
	shunt_view_t view = {
		.num_veh = count,
		.tgt_num = 0.7 * count,
		.stock = veh,
		.train = NULL,
	};
	view.train = safe_calloc(count, sizeof(veh_t *));
	shuffle_train(&view);
	
	do {
		shuntview_draw(&view);
	} while(shuntview_update(&view));
	
	free(view.train);
}
