/*===--------------------------------------------------------------------------------------------===
 * ui.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2024 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _UI_H_
#define _UI_H_

#include <term/hexes.h>
#include <term/colors.h>

void
ui_start();
void
ui_end();

void
ui_text(int w, const char *fmt, ...);
void
ui_line(const char *fmt, ...);
void
ui_title(const char *fmt, ...);
void
ui_prompt(const char *fmt, ...);

#endif /* ifndef _UI_H_ */


