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
	int		veh_count;
	const veh_t	*stock;
	const veh_t	*train;
} shunt_view_t;

void show_shuntview(db_t *db, const veh_t *veh, int count) {
	UNUSED(db_t *db);
}
