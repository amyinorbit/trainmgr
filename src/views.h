/*===--------------------------------------------------------------------------------------------===
 * views.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _VIEWS_H_
#define _VIEWS_H_

#include "stock.h"

void show_dbview(db_t *db);
void show_addview(db_t *db, veh_t *veh);
void show_shuntview(db_t *db, const veh_t **veh, int count);

#endif /* ifndef _VIEWS_H_ */


