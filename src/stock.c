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
#include <ctype.h>

static int
veh_cmp(const void *a, const void *b) {
	if(((const veh_t *)a)->num < ((const veh_t *)b)->num)
		return -1;
	if(((const veh_t *)a)->num > ((const veh_t *)b)->num)
		return 1;
	return 0;
}

typedef enum {
	LOK_ELEC 	= 1 << 0,
	LOK_DIESEL 	= 1 << 1,
	LOK_RACK 	= 1 << 2,
	LOK_NARROW 	= 1 << 3,
	
	PAX_FIRST	= 1 << 4,
	PAX_SECOND	= 1 << 5,
	PAX_RESTAURANT	= 1 << 6,
	
	PAX_PANORAMIC	= 1 << 7,
	
	LUGGAGE_VAN	= 1 << 8,
} veh_cap_mask_t;

static const char *
lok_traction_str(veh_type_t type, uint32_t mask) {
	if(type != VEH_TYPE_LOK && type != VEH_TYPE_RAILCAR) return NULL;
	if((mask & (LOK_ELEC|LOK_DIESEL)) == (LOK_ELEC|LOK_DIESEL))
		return "electro-diesel";
	if(mask & LOK_DIESEL)
		return "diesel";
	if(mask & LOK_ELEC)
		return "electric";
	return "steam";
}

static const char *
lok_gauge_str(uint32_t mask) {
	return (mask & LOK_NARROW) ? "narrow-gauge" : NULL;
}

static const char *
lok_rack_str(uint32_t mask) {
	return (mask & LOK_RACK) ? "rack" : NULL;
}

static const char *
coach_class_str(uint32_t mask) {
	if((mask & (PAX_FIRST|PAX_SECOND)) == (PAX_FIRST|PAX_SECOND))
		return "1st- and 2nd-class";
	if(mask & PAX_FIRST)
		return "1st-class";
	if(mask & PAX_SECOND)
		return "2nd-class";
	return NULL;
}

static const char *
coach_accessory_str(uint32_t mask) {
	return (mask & PAX_PANORAMIC) ? "panoramic" : NULL;
}

static const char *
van_str(veh_type_t type, uint32_t mask) {
	if(type == VEH_TYPE_VAN) return NULL;
	return (mask & (LUGGAGE_VAN)) ? "with luggage compartment" : NULL;	
}

static uint32_t
veh_uic_to_mask(char c) {
	switch(c) {
	case 'D': return LUGGAGE_VAN;
	case 'W': return PAX_RESTAURANT;
		
	case 'A': return PAX_FIRST;
	case 'B': return PAX_SECOND;
	
	case 'p': return PAX_PANORAMIC;
	
	case 'H': return LOK_RACK;
	case 'G': return LOK_NARROW;
	case 'e': return LOK_ELEC;
	
	case 'm': return LOK_DIESEL;
	case 'h': return LOK_RACK;
	}
	return 0;
}

static veh_type_t
veh_uic_to_type(char c) {
	switch(c) {
	case 'D':
		return VEH_TYPE_VAN;
	case 'W':
	case 'A':
	case 'B':
		return VEH_TYPE_COACH;
	case 'R':
	case 'G':
	case 'H':
	case 'e':
	case 'a':
	case 'f':
	case 'm':
	case 'h':
		return VEH_TYPE_LOK;
	case 'L':
		return VEH_TYPE_WAGON;
	}
	return VEH_TYPE_UNKNOWN;
}

static const char *type_names[] = {
	[VEH_TYPE_UNKNOWN] = "unknown",
	[VEH_TYPE_LOK] = "locomotive",
	[VEH_TYPE_VAN] = "luggage van",
	[VEH_TYPE_COACH] = "coach",
	[VEH_TYPE_WAGON] = "wagon",
	[VEH_TYPE_CONTROL] = "control",
	[VEH_TYPE_RAILCAR] = "railcar",
};

static void describe_mask(veh_type_t type, uint32_t mask, char *dest, int cap) {
	
	const char *comps[] = {
		coach_class_str(mask),
		coach_accessory_str(mask),
		
		lok_gauge_str(mask),
		lok_rack_str(mask),
		lok_traction_str(type, mask),
		
		type_names[type],
		
		van_str(type, mask),
	};
	
	bool before = false;
	for(int i = 0; i < 7; ++i) {
		if(!comps[i]) continue;
		size_t written = snprintf(dest, cap, "%s%s", before ? " " : "", comps[i]);
		cap -= written;
		dest += written;
		before = true;
		if(!cap) break;
	}
}

static void
veh_find_type(veh_t *veh) {
	uint32_t type_mask = 0;
	veh_type_t type = VEH_TYPE_UNKNOWN;
	UNUSED(type_names);
	
	const char *c = veh->class;
	while(*c && !isspace(*c)) {
		veh_type_t new_type = veh_uic_to_type(*c);
		type_mask |= veh_uic_to_mask(*c);
		c++;
		switch(type) {
		case VEH_TYPE_UNKNOWN:
			type = new_type;
			break;
		case VEH_TYPE_LOK:
			type = new_type > VEH_TYPE_LOK ? VEH_TYPE_RAILCAR : VEH_TYPE_LOK;
			break;
		default:
			if(new_type == VEH_TYPE_LOK) {
				type = VEH_TYPE_RAILCAR;
			} else {
				type = MAX(new_type, type);
			}
			break;
		}
	}
	veh->type = type;
	describe_mask(type, type_mask, veh->class_desc, sizeof(veh->class_desc));
}

static void
post_proc_veh(veh_t *veh) {
	snprintf(veh->combo_desc, sizeof(veh->combo_desc), "%s %d", veh->class, veh->num);
	veh_find_type(veh);
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

	post_proc_veh(veh);
	
	avl_index_t where;
	if(avl_find(db, veh, &where) != NULL)
		return false;
	avl_insert(db, veh, where);
	return true;
}

bool
stock_db_update(db_t *db, veh_t *veh) {
	ASSERT(db != NULL);
	ASSERT(veh != NULL);
	post_proc_veh(veh);
	return avl_update(db, veh);
}

veh_t *
stock_db_get(const avl_tree_t *db, int num) {
	ASSERT(db != NULL);
	
	veh_t search = {.num = num};
	return avl_find(db, &search, NULL);
}

void
stock_db_delete(db_t *db, veh_t *veh) {
	ASSERT(db != NULL);
	ASSERT(veh != NULL);
	
	avl_remove(db, veh);
	free(veh);
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
parse_one_veh(char **comps, int offset) {
	veh_t *veh = safe_calloc(1, sizeof(*veh));
	if(offset)
		veh->in_use = comps[0][0] == 'x';
	else
		veh->in_use = false;
	
	veh->num = atoi(comps[offset+0]);
	
	str_trim_space(comps[offset+1]);
	strncpy(veh->class, comps[offset+1], sizeof(veh->class));
	str_trim_space(comps[offset+2]);
	strncpy(veh->desc, comps[offset+2], sizeof(veh->desc));
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
		
		char *comps[4];
        	unsigned n_comps = str_split_inplace(line, ',', comps, 4);
		if(n_comps < 3)
			continue;
		
		veh_t *veh = parse_one_veh(comps, n_comps > 3);
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
		fprintf(f, "%c,%d, %s, %s\n",
			veh->in_use ? 'x' : '-',
			veh->num,
			veh->class,
			veh->desc);
	}
	return true;
}

