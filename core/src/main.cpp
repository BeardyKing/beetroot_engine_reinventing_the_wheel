#include <beet/window.h>

int main() {
    window_create();

    while (window_is_open()) {
        window_poll();
    }

    window_cleanup();
    return 0;
}
