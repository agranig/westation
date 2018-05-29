#ifndef _DISPLAY_H
#define _DISPLAY_H

int display_init(const char *name, int w, int h);

void display_destroy();

int display_show(const char* icon_path, const char* text, int temp, int hum);

#endif // _DISPLAY_H
