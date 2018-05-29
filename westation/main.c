#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "weather.h"
#include "display.h"
#include "eventloop.h"

#define ICON_NA_PATH "assets/meteocons-icons/PNG/44.png"

pthread_mutex_t g_lock;
int g_quit = 0;

static void* run_weather(void *ptr) {
    weather_info_t info;
    int quit = 0;

    do {
        memset(&info, 0, sizeof(info));

        if (weather_get(&info) != 0) {
            fprintf(stderr, "Failed to fetch data from weather API\n");
            goto err;
        }

        fprintf(stdout, 
                "Summary:\n" \
                "  Description: %s\n" \
                "  Icon:        %s\n",
                info.description,
                info.icon
        );

        pthread_mutex_lock(&g_lock);
        display_show(info.icon, info.description);
        quit = g_quit;
        pthread_mutex_unlock(&g_lock);

        weather_destroy_info(&info);
        goto pause;

err:
        pthread_mutex_lock(&g_lock);
        display_show(ICON_NA_PATH, "Weather API Error");
        pthread_mutex_unlock(&g_lock);

pause:
        usleep(10000000);
    } while (!quit);

    return NULL;
}

int main(int argc, const char** argv) {
    int ret;

    pthread_t weather_thread, display_thread;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <api-key>\n", argv[0]);
        return -1;
    }

    if (pthread_mutex_init(&g_lock, NULL) != 0) {
        fprintf(stderr, "Failed to init mutex\n");
        ret = -1; goto out;
    }
   
    if  (display_init("westation", 480, 320) != 0) {
        fprintf(stderr, "Failed to init display\n");
        ret = -1; goto out;
    }

    if  (weather_init(argv[1], "1100", "AT", "metric", "de") != 0) {
        fprintf(stderr, "Failed to init weather API\n");
        ret = -1; goto out;
    }

    // TODO: use another mutex inside display_show to avoid manual locking
    pthread_mutex_lock(&g_lock);
    display_show(ICON_NA_PATH, "Initializing...");
    pthread_mutex_unlock(&g_lock);

    if (pthread_create(&weather_thread, NULL, run_weather, NULL) != 0) {
        fprintf(stderr, "Failed to init weather thread\n");
        ret = -1; goto out;
    }

    eventloop_run();

    pthread_mutex_lock(&g_lock);
    g_quit = 1;
    pthread_mutex_unlock(&g_lock);

    if (pthread_join(weather_thread, NULL) != 0) {
        fprintf(stderr, "Failed to join weather thread\n");
        ret = -1; goto out;
    }

out:

    weather_destroy();
    display_destroy();

    return ret;
}

