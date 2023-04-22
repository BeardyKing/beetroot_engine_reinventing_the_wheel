#include <beet/window.h>
#include <beet/time.h>
#include <cstdio>
#include <beet/input.h>

int main() {
    window_create();
    time_create();
    input_create();

    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());
        window_poll();

        if (input_key_pressed(KeyCode::W)) {
            printf("TIME: %f \n", time_current());
            printf("FRAME: %u\n", time_frame_count());
            printf("keycode: [%c] is pressed\n", (char) KeyCode::W);
        }
        float timeHeldDown = input_key_down_time(KeyCode::W);
        if (timeHeldDown > 0 || input_key_down(KeyCode::W)) {
            printf("%f\n", timeHeldDown);
        }
    }

    input_cleanup();
    time_cleanup();
    window_cleanup();
    return 0;
}
