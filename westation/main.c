#include <stdio.h>

#include "weather.h"

int main(int argc, const char** argv) {
    int ret;
    weather_info_t info;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <api-key>\n", argv[0]);
        return -1;
    }
   
    if  (weather_init(argv[1], "1100", "AT", "metric", "de") != 0) {
        fprintf(stderr, "Failed to init weather API\n");
        ret = -1; goto out;
    }

    if (weather_get(&info) != 0) {
        fprintf(stderr, "Failed to fetch data from weather API\n");
        ret = -1; goto out;
    }

    fprintf(stdout, 
            "Summary:\n" \
            "  Description: %s\n" \
            "  Icon:        %s\n",
            info.description,
            info.icon
    );

out:
    if (weather_destroy() != 0) {
        fprintf(stderr, "Failed to cleanup weather API\n");
        return -1;
    }

    weather_destroy_info(&info);

    return ret;
}

