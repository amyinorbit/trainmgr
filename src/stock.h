/*===--------------------------------------------------------------------------------------------===
 * stock.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _STOCK_H_
#define _STOCK_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <utils/avl.h>

#define MAX_CLASS_LEN	(16)
#define MAX_DESC_LEN	(32)

typedef avl_tree_t	db_t;

typedef enum {
	CLASS_FIRST_CLASS	= 'A',
	CLASS_STD_CLASS		= 'B',
	CLASS_VAN		= 'D',
	CLASS_PANORAMIC		= 'p',
	
	CLASS_CONTROL		= 't',
	
	CLASS_MET_LOK		= 'G',
	CLASS_ELEC_LOK		= 'e',
	CLASS_RACK_LOK		= 'h'
} class_t;


typedef struct {
	int		num;
	char		class[MAX_CLASS_LEN];
	char		desc[MAX_DESC_LEN];
	avl_node_t	db_node;
} veh_t;

void
stock_db_init(db_t *db);

void
stock_db_fini(db_t *db);

bool
stock_db_add(db_t *db, veh_t *veh);

bool
stock_db_update(db_t *db, veh_t *veh);

veh_t *
stock_db_get(const db_t *db, int num);

bool
stock_db_delete(db_t *db, veh_t *veh);

static inline size_t
stock_db_get_count(const db_t *db) {
	return avl_numnodes(db);
}

size_t
stock_db_get_list(const db_t *db, int *list, size_t cap);

ssize_t
stock_load_from_path(const char *path, db_t *db);

ssize_t
stock_load_from_file(FILE *f, db_t *db);

bool
stock_write_to_path(const char *path, const db_t *db);

bool
stock_write_to_file(FILE *f, const db_t *db);

#endif /* ifndef _STOCK_H_ */


