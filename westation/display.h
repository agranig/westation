#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "weather.h"

int display_init(const char *name, int w, int h);

void display_destroy();

// TODO: make these generic

// IMPORTANT: onlyu call display_show from main thread
int display_show(const char* icon_path, const char* text, int temp, int hum);

// since rendering via SDL only works reliable on both macos and raspi when drawing
// from same thread which initialized SDL, we've to queue up changes in worker
// threads and draw them in the main thread
int display_queue(weather_info_t *info);
int display_process_queue();


#endif // _DISPLAY_H
