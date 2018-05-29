#include <stdio.h>
#include <unistd.h>

#include "weather.h"
#include "display.h"

int main(int argc, const char** argv) {
    int ret;
    weather_info_t info;

    memset(&info, 0, sizeof(info));

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <api-key>\n", argv[0]);
        return -1;
    }
   
    if  (display_init("westation", 480, 320) != 0) {
        fprintf(stderr, "Failed to init display\n");
        ret = -1; goto out;
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

    display_show(info.icon, info.description);

out:
    weather_destroy_info(&info);

    weather_destroy();
    display_destroy();

    return ret;
}

