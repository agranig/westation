#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "weather.h"
#include "display.h"
#include "eventloop.h"

#define ICON_NA_PATH "assets/icons/45.png"
#define ICON_INIT_PATH "assets/icons/44.png"

pthread_mutex_t g_lock;
int g_quit = 0;

static void* run_weather(void *ptr) {
    weather_info_t info;
    int quit = 0;
    int query_interval_us = 60000000;
    int pause_us = 100000;
    int runtime = query_interval_us;

    do {
        if (runtime >= query_interval_us) {
            runtime = 0;

            memset(&info, 0, sizeof(info));

            if (weather_get(&info) != 0) {
                fprintf(stderr, "Failed to fetch data from weather API\n");
                goto pause;
            }

            fprintf(stdout, 
                    "Summary:\n" \
                    "  Temperature: %d°\n" \
                    "  Humidity:    %d%%\n" \
                    "  Description: %s\n" \
                    "  Icon:        %s\n",
                    (int)info.temperature,
                    info.humidity,
                    info.description,
                    info.icon
            );

            if (display_queue(&info) != 0) {
                fprintf(stderr, "failed to queue weather info\n");
				goto pause;
            }

            weather_destroy_info(&info);
        }

pause:
        pthread_mutex_lock(&g_lock);
        quit = g_quit;
        pthread_mutex_unlock(&g_lock);

        runtime += pause_us;
        usleep(pause_us);
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

    if (display_show(ICON_INIT_PATH, "Initializing...", 0, 0) != 0) {
        fprintf(stderr, "Failed to render display\n");
        pthread_mutex_unlock(&g_lock);
        goto out;
    }

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
    pthread_mutex_destroy(&g_lock);

    return ret;
}

