/*===--------------------------------------------------------------------------------------------===
 * main.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "stock.h"
#include "views.h"
#include <utils/helpers.h>


int main(int argc, const char **argv) {
	if(argc < 2)
		return -1;
	
	const char *db_path = argc >= 2 ? argv[1] : "";
	
	avl_tree_t db;
	stock_db_init(&db);
	
	stock_load_from_path(db_path, &db);
	
	show_menu(&db);
	
	stock_write_to_path(db_path, &db);
	stock_db_fini(&db);
}


