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
#include <utils/helpers.h>

void show_menu(db_t *db);
void show_dbview(db_t *db);

int main(int argc, const char **argv) {
	if(argc < 2)
		return -1;
	
	const char *db_path = argc >= 2 ? argv[1] : "";
	
	avl_tree_t db;
	stock_db_init(&db);
	
	ssize_t count = stock_load_from_path(db_path, &db);
	if(count < 0) {
		fprintf(stderr, "error reading rolling stock database\n");
		return -1;
	}
	
	show_dbview(&db);
	// int *running_nums = safe_calloc(count, sizeof(int));
	// stock_db_get_list(&db, running_nums, count);
	// for(ssize_t i = 0; i < count; ++i) {
	// 	fprintf(stderr, "%d\n", running_nums[i]);
	// }
	// free(running_nums);
	
	stock_write_to_path("test.csv", &db);
	stock_db_fini(&db);
}


