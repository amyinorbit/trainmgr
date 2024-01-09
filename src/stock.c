/*===--------------------------------------------------------------------------------------------===
 * stock.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "stock.h"
#include <stdio.h>
#include <utils/assert.h>
#include <utils/helpers.h>

static int veh_cmp(const void *a, const void *b) {
	if(((const veh_t *)a)->num < ((const veh_t *)b)->num)
		return -1;
	if(((const veh_t *)a)->num > ((const veh_t *)b)->num)
		return 1;
	return 0;
}

void
stock_db_init(avl_tree_t *db) {
	ASSERT(db != NULL);
	
	avl_create(db, veh_cmp, sizeof(veh_t), offsetof(veh_t, db_node));
}

void
stock_db_fini(avl_tree_t *db) {
	ASSERT(db != NULL);
	
	veh_t *veh = NULL;
	void *cookie = NULL;
	
	while((veh = avl_destroy_nodes(db, &cookie)) != NULL) {
		free(veh);
	}
	avl_destroy(db);
}

bool
stock_db_add(avl_tree_t *db, veh_t *veh) {
	ASSERT(db != NULL);
	ASSERT(veh != NULL);
	
	avl_index_t where;
	if(avl_find(db, veh, &where) != NULL)
		return false;
	avl_insert(db, veh, where);
	return true;
}

veh_t *
stock_db_get(const avl_tree_t *db, int num) {
	ASSERT(db != NULL);
	
	veh_t search = {.num = num};
	return avl_find(db, &search, NULL);
}

size_t
stock_db_get_list(const avl_tree_t *db, int *list, size_t cap) {
	ASSERT(db != NULL);
	
	size_t written = 0;
	for(const veh_t *veh = avl_first(db); veh; veh = AVL_NEXT(db, veh)) {
		if(written >= cap) break;
		list[written++] = veh->num;
	}
	return written;
}

ssize_t
stock_load_from_path(const char *path, avl_tree_t *db) {
	ASSERT(db != NULL);
	ASSERT(path != NULL);
	
	FILE *f = fopen(path, "rb");
	if(!f)
		return -1;
	return stock_load_from_file(f, db);
}

static veh_t *
parse_one_veh(char *comps[3]) {
	veh_t *veh = safe_calloc(1, sizeof(*veh));
	veh->num = atoi(comps[0]);
	
	str_trim_space(comps[1]);
	strncpy(veh->class, comps[1], sizeof(veh->class));
	str_trim_space(comps[2]);
	strncpy(veh->desc, comps[2], sizeof(veh->desc));
	return veh;
}

ssize_t
stock_load_from_file(FILE *f, avl_tree_t *db) {
	ASSERT(db != NULL);
	ASSERT(f != NULL);
	
        char *line = NULL;
        size_t cap = 0;
	size_t count = 0;
        while(getline(&line, &cap, f) > 0) {
		str_trim_space(line);
		if(line[0] == '#')
			continue;
		
		char *comps[3];
        	unsigned n_comps = str_split_inplace(line, ',', comps, 3);
		if(n_comps < 3)
			continue;
		
		veh_t *veh = parse_one_veh(comps);
		if(!veh)
			continue;
		if(!stock_db_add(db, veh))
			continue;
		count += 1;
        }
	if(line && cap)
		free(line);
	
	return (ssize_t)count;
}

bool
stock_write_to_path(const char *path, const avl_tree_t *db) {
	ASSERT(db != NULL);
	ASSERT(path != NULL);
	
	FILE *f = fopen(path, "wb");
	if(!f)
		return false;
	return stock_write_to_file(f, db);
}

bool
stock_write_to_file(FILE *f, const avl_tree_t *db) {
	ASSERT(db != NULL);
	ASSERT(f != NULL);
	
	for(const veh_t *veh = avl_first(db); veh; veh = AVL_NEXT(db, veh)) {
		fprintf(f, "%d, %s, %s\n", veh->num, veh->class, veh->desc);
	}
	return true;
}

