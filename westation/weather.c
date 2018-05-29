#include <curl/curl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "weather.h"

#define QUERY_TMPL "https://api.openweathermap.org/data/2.5/weather?" \
                          "APPID=%s&zip=%s,%s&units=%s&lang=%s"

// TODO: to be defined in config
#define ICON_ASSET_PATH "assets/meteocons-icons/PNG"

CURL *g_curl = NULL;

typedef struct read_buffer {
    char *data;
    size_t size;
} weather_buffer_t;

int weather_init(const char* key, const char* zip, const char* country,
        const char* units, const char* lang) {
    char query[1024] = "";

    if(g_curl) {
        return 0;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    g_curl = curl_easy_init();
    if (!g_curl) {
        return -1;
    }
    if(snprintf(query, sizeof(query), QUERY_TMPL, key, zip, country, units, lang) >= sizeof(query)) {
        return -1;
    }
    curl_easy_setopt(g_curl, CURLOPT_URL, query);
    curl_easy_setopt(g_curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    return 0;
}

void weather_destroy() {
    if (!g_curl)
        return;
    
    curl_easy_cleanup(g_curl);
    g_curl = NULL;
    curl_global_cleanup();
}

weather_info_t* weather_dup_info(weather_info_t *info) {
    weather_info_t *new_info;

    if (!info)
        return NULL;

    new_info = malloc(sizeof(weather_info_t));
    if (!new_info) {
        fprintf(stderr, "Failed to allocate weather info dup memory\n");
        goto err;
    }

    memcpy(new_info, info, sizeof(*new_info));
    new_info->description = NULL;
    new_info->icon = NULL;

    new_info->description = strdup(info->description);
    if (!new_info->description) {
        fprintf(stderr, "Failed to allocate weather info dup memory\n");
        goto err;
    }

    new_info->icon = strdup(info->icon);
    if (!new_info->icon) {
        fprintf(stderr, "Failed to allocate weather info dup memory\n");
        goto err;
    }

    return new_info;

err:
    weather_destroy_info(new_info);
    if (new_info)
        free(new_info);
    return NULL;
}

void weather_destroy_info(weather_info_t *info) {
    if (!info)
        return;

    if (info->description) {
        free(info->description);
        info->description = NULL;
    }
    if (info->icon) {
        free(info->icon);
        info->icon= NULL;
    }
}

static size_t weather_write_buffer(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    weather_buffer_t *buffer = (weather_buffer_t*)userp;

    buffer->data = realloc(buffer->data, buffer->size + realsize + 1);
    if (!buffer->data) {
        fprintf(stderr, "Failed to allocate buffer memory\n");
        return 0;
    }

    memcpy(&(buffer->data[buffer->size]), contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = 0;

    return realsize;
}

static char* weather_type_to_icon(weather_type_t type, weather_time_t time) {
    // TODO: move to config file to avoid code change on file name change
    switch (time) {
        case WEATHER_DAY: {
            switch (type) {
                case WEATHER_CLEAR:
                    return "2.png";

                case WEATHER_FEWCLOUDS:
                    return "8.png";

                case WEATHER_SCATTEREDCLOUDS:
                    return "14.png";

                case WEATHER_BROKENCLOUDS:
                    return "25.png";

                case WEATHER_SHOWERRAIN:
                    return "18.png";

                case WEATHER_RAIN:
                    return "17.png";

                case WEATHER_THUNDERSTORM:
                    return "16.png";
            
                case WEATHER_SNOW:
                    return "22.png";

                case WEATHER_MIST:
                    return "13.png";

                default:
                    return "45.png";
            }
        }
        case WEATHER_NIGHT: {
            switch (type) {
                case WEATHER_CLEAR:
                    return "29.png";

                case WEATHER_FEWCLOUDS:
                    return "31.png";

                case WEATHER_SCATTEREDCLOUDS:
                    return "32.png";

                case WEATHER_BROKENCLOUDS:
                    return "41.png";

                case WEATHER_SHOWERRAIN:
                    return "35.png";

                case WEATHER_RAIN:
                    return "34.png";

                case WEATHER_THUNDERSTORM:
                    return "33.png";
            
                case WEATHER_SNOW:
                    return "38.png";

                case WEATHER_MIST:
                    return "36.png";

                default:
                    return "45.png";
            }
        }
        default:
            return "45.png";
    }
}

static weather_type_t weather_code_to_type(int code) {
    // TODO: move to config file to avoid code change on API change
    switch (code) {
        case 200:
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221:
        case 230:
        case 231:
        case 232:
            return WEATHER_THUNDERSTORM;

        case 300:
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321:
            return WEATHER_SHOWERRAIN;

        case 500:
        case 501:
        case 502:
        case 503:
        case 504:
            return WEATHER_RAIN;

        case 511:
            return WEATHER_SNOW;

        case 520:
        case 521:
        case 522:
        case 531:
            return WEATHER_SHOWERRAIN;

        case 600:
        case 601:
        case 602:
        case 611:
        case 612:
        case 615:
        case 616:
        case 620:
        case 621:
        case 622:
            return WEATHER_SNOW;

        case 701:
        case 711:
        case 721:
        case 731:
        case 741:
        case 751:
        case 761:
        case 762:
        case 771:
        case 781:
            return WEATHER_MIST;

        case 800:
            return WEATHER_CLEAR;

        case 801:
            return WEATHER_FEWCLOUDS;

        case 802:
            return WEATHER_SCATTEREDCLOUDS;

        case 803:
        case 804:
            return WEATHER_BROKENCLOUDS;

        default:
            return WEATHER_UNKNOWN;
    }
}

static weather_time_t weather_sunrise_to_time(int sunrise, int sunset, int now) {
    if (now >= sunrise && now < sunset)
        return WEATHER_DAY;
    else
        return WEATHER_NIGHT;
}

int weather_get(weather_info_t *info) {
    CURLcode res;
    struct json_object *json_root;
    struct json_object *json_sys;
    struct json_object *json_main;
    struct json_object *json_weathers;
    struct json_tokener *json_tok = NULL;
    weather_buffer_t buffer = {NULL, 0};
    char icon_path_buffer[2048];

    if (!g_curl) {
        fprintf(stderr, "Uninitialized g_curl object, weather_init() not called?\n");
        goto err;
    }

    curl_easy_setopt(g_curl, CURLOPT_WRITEFUNCTION, weather_write_buffer);
    curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, (void *)&buffer);
    res = curl_easy_perform(g_curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to perform API request: %s\n",
              curl_easy_strerror(res));
        goto err;
    }

    if (!buffer.data) {
        fprintf(stderr, "API returned no data\n");
        goto err;
    }

    fprintf(stdout, "Got weather data: %s\n", buffer.data);

    // TODO: move whole json parsing to a function, and return a weather struct from there
   
    json_tok = json_tokener_new(); 
    json_root = json_tokener_parse_ex(json_tok, buffer.data, buffer.size);
    if (!json_root) {
        fprintf(stderr, "Failed to parse json response: %s\n",
                json_tokener_error_desc(json_tokener_get_error(json_tok)));
        goto err;
    }
    
    if (json_object_object_get_ex(json_root, "weather", &json_weathers)) {
        if (!json_object_is_type(json_weathers, json_type_array)) {
            fprintf(stderr, "Key 'weather' is not an array\n");
            goto err;
        }
        for (int i = 0; i < json_object_array_length(json_weathers); ++i) {
            // TODO: why are there sometimes more than 1 entries in the array,
            // how to interpret this?
            struct json_object *json_weather_desc;
            struct json_object *json_weather_code;
            struct json_object *json_weather =
                json_object_array_get_idx(json_weathers, i);

            if (json_object_object_get_ex(json_weather, "description", &json_weather_desc)) {
                const char* desc;
                if (!json_object_is_type(json_weather_desc, json_type_string)) {
                    fprintf(stderr, "Key 'weather[%d].description' is not a string\n", i);
                    goto err;
                }
                desc = json_object_get_string(json_weather_desc);
                if (desc) {
                    info->description = strdup(desc);
                    if (!info->description) {
                        fprintf(stderr, "Failed to allocate description memory\n");
                        goto err;
                    }
                } else {
                    info->description = strdup("N/A");
                }
            } else {
                fprintf(stderr, "Key 'weather[%d].description' not found in API response\n", i);
                goto err;
            }

            if (json_object_object_get_ex(json_weather, "id", &json_weather_code)) {
                int code;
                if (!json_object_is_type(json_weather_code, json_type_int)) {
                    fprintf(stderr, "Key 'weather[%d].id' is not an int\n", i);
                    goto err;
                }
                errno = 0;
                code = json_object_get_int(json_weather_code);
                if (code == 0 && errno) {
                    fprintf(stderr, "Failed to get key 'weather[%d].id'\n", i);
                    goto err;
                } else {
                    info->type = weather_code_to_type(code);
                }
            } else {
                fprintf(stderr, "Key 'weather[%d].id' not found in API response\n", i);
                goto err;
            }

        }
    } else {
        fprintf(stderr, "Key 'weather' not found in API response\n");
        goto err;
    }

    if (json_object_object_get_ex(json_root, "sys", &json_sys)) {
        json_object *json_sys_time;
        int sunrise, sunset;
        if (json_object_object_get_ex(json_sys, "sunrise", &json_sys_time)) {
            int time;
            if (!json_object_is_type(json_sys_time, json_type_int)) {
                fprintf(stderr, "Key 'sys.sunrise' is not an int\n");
                goto err;
            }
            errno = 0;
            time = json_object_get_int(json_sys_time);
            if (time == 0 && errno) {
                fprintf(stderr, "Failed to get key 'sys.sunrise'\n");
                goto err;
            } else {
                sunrise = time;
            }
        } else {
            fprintf(stderr, "Key 'sys.sunrise' not found in API response\n");
            goto err;
        }

        if (json_object_object_get_ex(json_sys, "sunset", &json_sys_time)) {
            int time;
            if (!json_object_is_type(json_sys_time, json_type_int)) {
                fprintf(stderr, "Key 'sys.sunset' is not an int\n");
                goto err;
            }
            errno = 0;
            time = json_object_get_int(json_sys_time);
            if (time == 0 && errno) {
                fprintf(stderr, "Failed to get key 'sys.sunset'\n");
                goto err;
            } else {
                sunset = time;
            }
        } else {
            fprintf(stderr, "Key 'sys.sunset' not found in API response\n");
            goto err;
        }
        info->time = weather_sunrise_to_time(sunrise, sunset, time(NULL));
    } else {
        fprintf(stderr, "Key 'sys' not found in API response\n");
        goto err;
    }

    if (json_object_object_get_ex(json_root, "main", &json_main)) {
        json_object *json_sys_temp;
        json_object *json_sys_humidity;
        if (json_object_object_get_ex(json_main, "temp", &json_sys_temp)) {
            if (!json_object_is_type(json_sys_temp, json_type_double)) {
                fprintf(stderr, "Key 'main.temp' is not a double\n");
                goto err;
            }
            errno = 0;
            info->temperature = json_object_get_int(json_sys_temp);
            if (info->temperature == 0 && errno) {
                fprintf(stderr, "Failed to get key 'main.temp'\n");
                goto err;
            }
        } else {
            fprintf(stderr, "Key 'main.temp' not found in API response\n");
            goto err;
        }

        if (json_object_object_get_ex(json_main, "humidity", &json_sys_humidity)) {
            if (!json_object_is_type(json_sys_humidity, json_type_int)) {
                fprintf(stderr, "Key 'main.humidity' is not an double\n");
                goto err;
            }
            errno = 0;
            info->humidity = json_object_get_double(json_sys_humidity);
            if (info->humidity == 0 && errno) {
                fprintf(stderr, "Failed to get key 'main.humidity'\n");
                goto err;
            }
        } else {
            fprintf(stderr, "Key 'main.humidity' not found in API response\n");
            goto err;
        }
    } else {
        fprintf(stderr, "Key 'main' not found in API response\n");
        goto err;
    }

    if (snprintf(icon_path_buffer, sizeof(icon_path_buffer), "%s/%s",
            ICON_ASSET_PATH,
            weather_type_to_icon(info->type, info->time)) >= sizeof(icon_path_buffer)) {
        fprintf(stderr, "Failed to set icon path buffer, too small buffer size\n");
        goto err;
    }
    info->icon = strdup(icon_path_buffer);
        
    json_tokener_free(json_tok);
    free(buffer.data);
    return 0;

err:
    if (json_tok) {
        json_tokener_free(json_tok);
    }

    if (buffer.data) {
        free(buffer.data);
    }

    weather_destroy_info(info);

    return -1;
}
