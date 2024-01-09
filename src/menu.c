/*===--------------------------------------------------------------------------------------------===
 * menu.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "stock.h"
#include "ui.h"
#include <utils/helpers.h>

typedef struct {
	int width;
	int height;
	int num_items;
	int sel_item;
	const char **items;
} menu_data_t;

static void menu_draw_items(const menu_data_t *menu) {
	int start_y = (menu->height / 2) - (menu->num_items / 2);
	
	int longest = 0;
	for(int i = 0; i < menu->num_items; ++i) {
		longest = MAX(longest, (int)strlen(menu->items[i]));
	}

	int start_x = (menu->width / 2) - (longest / 2) - 3;
	longest = MIN(longest, menu->width-4);
	
	for(int i = 0; i < menu->num_items; ++i) {
		hexes_cursor_go(start_x, start_y + i);
		
		if(i == menu->sel_item)
			term_reverse(stdout);
		printf(" * %-*s ", longest, menu->items[i]);
		term_style_reset(stdout);
	}
}

static void menu_draw(menu_data_t *menu) {
	hexes_get_size(&menu->width, &menu->height);
	hexes_clear_screen();
	
	ui_title(" Rolling Stock Database 1.0");
	menu_draw_items(menu);
	ui_prompt(" Up/Down: move cursor     Return: select     Ctrl-C: exit");
}

static bool menu_update(menu_data_t *menu) {
	menu_draw(menu);
	
	int c = hexes_get_key_raw();
	switch(c) {
	case KEY_CTRL_C:
	case KEY_CTRL_D:
	case KEY_CTRL_Q:
	case KEY_ESC:
	case 'q':
		return false;
		
	case KEY_ARROW_UP:
		menu->sel_item = MAX(0, menu->sel_item-1);
		break;
	case KEY_ARROW_DOWN:
		menu->sel_item = MIN(menu->num_items-1, menu->sel_item+1);
		break;
	default:
		break;
	}
	
	return true;
}

static const char *options[] = {
	"Query Database",
	"Add Rolling Stock",
	"Shunting Puzzle"
};


void show_menu(db_t *db) {
	UNUSED(db);
	
	menu_data_t menu;
	menu.num_items = 3;
	menu.sel_item = 0;
	menu.items = options;
		
	ui_start();
	
	while(menu_update(&menu)) {}
	
	ui_end();
}
