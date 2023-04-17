#include <beet/window.h>
#include <beet/time.h>
#include <cstdio>

int main() {
    window_create();
    time_create();

    while (window_is_open()) {
        time_tick();
        window_poll();

        time_tick();
        
        printf("DT: %lf \n", time_delta());
    }

    time_cleanup();
    window_cleanup();
    return 0;
}
