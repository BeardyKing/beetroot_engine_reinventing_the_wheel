#include <beet/window.h>
#include <beet/time.h>
#include <cstdio>
#include <beet/input.h>
#include <beet/maths/vec2.h>

void input_example() {

    //keyboard
    {
        const KeyCode key = KeyCode::W;
        if (input_key_pressed(key)) {
            printf("TIME: %f \n", time_current());
            printf("FRAME: %u\n", time_frame_count());
            printf("keycode: [%c] is pressed\n", (char) key);
        }
        float timeHeldDown = input_key_down_time(key);
        if (timeHeldDown > 0 || input_key_down(key)) {
            printf("%f\n", timeHeldDown);
        }
    }

    //mouse move
    {
        if (input_mouse_down(MouseButton::Left)) {
            vec2i pos = input_mouse_position();
            vec2f mouseDelta = input_mouse_delta();
            vec2f mouseDeltaRaw = input_mouse_delta_raw();
            if (mouseDeltaRaw.x != 0.0f || mouseDeltaRaw.y != 0.0f) {
                printf("POS    : [ %i , %i ]\n", pos.x, pos.y);
                printf("DELTA R: [ %f , %f ]\n", mouseDelta.x, mouseDelta.y);
                printf("DELTA  : [ %f , %f ]\n", mouseDeltaRaw.x, mouseDeltaRaw.y);
            }
        }
    }

    //mouse scroll
    {
        float scroll = input_mouse_scroll_delta();
        float scrollRaw = input_mouse_scroll_delta_raw();
        if (scrollRaw != 0) {
            printf("SCROLL DELTA R %f:\n", scrollRaw);
            printf("SCROLL DELTA   %f:\n", scroll);
        }
    }

    //cursor modes
    {
        if (input_mouse_pressed(MouseButton::Right)) {
            printf("TIME: %f \n", time_current());

            window_set_cursor_lock_position(input_mouse_position());
            window_set_cursor(CursorState::HiddenLockedLockMousePos);
        }
        if (input_mouse_released(MouseButton::Right)) {
            printf("TIME: %f \n", time_current());
            window_set_cursor(CursorState::Normal);
        }
    }

    //cursor window
    {
        if (input_key_down(KeyCode::Space)) {
            if (window_is_cursor_over_window()) {
                printf("cursor over main window\n");
            } else {
                printf("cursor is free\n");
            };
        }
    }

    //mouse button
    {
        const MouseButton mouseButton = MouseButton::Back;
        if (input_mouse_pressed(mouseButton)) {
            printf("TIME: %f \n", time_current());
            printf("FRAME: %u\n", time_frame_count());
            printf("keycode: [%c] is pressed\n", (char) mouseButton);
        }
        float mouseTimeHeldDown = input_mouse_down_time(mouseButton);
        if (mouseTimeHeldDown > 0 || input_mouse_down(mouseButton)) {
            printf("%f\n", mouseTimeHeldDown);
        }
    }
}

int main() {
    window_create();
    time_create();
    input_create();

    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());

        window_update();
        input_update();

        input_example();
    }

    input_cleanup();
    time_cleanup();
    window_cleanup();
    return 0;
}
