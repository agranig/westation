#include <curl/curl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUERY_TMPL "https://api.openweathermap.org/data/2.5/weather?" \
                          "APPID=%s&zip=%s,%s&units=%s&lang=%s"

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

int weather_destroy() {
    if (!g_curl) {
        return -1;
    }
    curl_easy_cleanup(g_curl);
    g_curl = NULL;
    curl_global_cleanup();

    return 0;
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

int weather_get() {
    CURLcode res;
    struct json_object *json_root;
    struct json_object *json_weathers;
    struct json_tokener *json_tok = NULL;
    weather_buffer_t buffer = {NULL, 0};

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
            struct json_object *json_weather_desc;
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
                    fprintf(stdout, "Description: %s\n", desc);
                }
            } else {
                fprintf(stderr, "Key 'weather[%d].description' not found in API response\n", i);
                goto err;
            }
        }
    } else {
        fprintf(stderr, "Key 'weather' not found in API response\n");
        goto err;
    }
        
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
    return -1;
}

int main(int argc, const char** argv) {
    int ret;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <api-key>\n", argv[0]);
        return -1;
    }
   
    if  (weather_init(argv[1], "1100", "AT", "metric", "de") != 0) {
        fprintf(stderr, "Failed to init weather API\n");
        ret = -1; goto out;
    }

    if (weather_get() != 0) {
        fprintf(stderr, "Failed to fetch data from weather API\n");
        ret = -1; goto out;
    }

out:
    if  (weather_destroy() != 0) {
        fprintf(stderr, "Failed to cleanup weather API\n");
        return -1;
    }
    return ret;
}

