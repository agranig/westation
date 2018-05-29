#ifndef _WEATHER_H
#define _WEATHER_H

#include <string.h>

typedef enum {
    WEATHER_UNKNOWN,
    WEATHER_CLEAR,
    WEATHER_FEWCLOUDS,
    WEATHER_SCATTEREDCLOUDS,
    WEATHER_BROKENCLOUDS,
    WEATHER_SHOWERRAIN,
    WEATHER_RAIN,
    WEATHER_THUNDERSTORM,
    WEATHER_SNOW,
    WEATHER_MIST
} weather_type_t;

typedef enum {
    WEATHER_NIGHT,
    WEATHER_DAY
} weather_time_t;

typedef struct weather_info {
    weather_type_t type;
    weather_time_t time;
    char *description;
    char *icon;
} weather_info_t;

int weather_init(const char* key, const char* zip, const char* country,
        const char* units, const char* lang);

void weather_destroy();

void weather_destroy_info(weather_info_t *info);

int weather_get(weather_info_t *info);

#endif // _WEATHER_H
