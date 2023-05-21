#include <core/engine.h>
#include <core/window.h>
#include <core/time.h>
#include <core/input.h>
#include <net/examples/sockets_examples.h>
#include <shared/log.h>
#include <gfx/gfx_interface.h>

void client_setup_system_orders() {
    engine_register_system_create(0, window_create);
    engine_register_system_create(1, time_create);
    engine_register_system_create(2, input_create);
    engine_register_system_create(3, []() {
        gfx_create();
        gfx_create_instance();
        window_create_render_surface(gfx_instance(), gfx_surface());
        gfx_create_debug_callbacks();
        gfx_create_physical_device();
    });

    engine_register_system_update(0, time_tick);
    engine_register_system_update(1, []() { input_set_time(time_current()); });
    engine_register_system_update(2, window_update);
    engine_register_system_update(3, input_update);

    //executed as reverse iter
    engine_register_system_cleanup(0, window_cleanup);
    engine_register_system_cleanup(1, time_cleanup);
    engine_register_system_cleanup(2, input_cleanup);
    engine_register_system_cleanup(3, []() {
        gfx_cleanup_physical_device();
        gfx_cleanup_debug_callbacks();
        gfx_cleanup_surface();
        gfx_cleanup_instance();
        gfx_cleanup();
    });
}

int main() {
    engine_create();
    client_setup_system_orders();
    engine_system_create();

    while (engine_is_open()) {
        engine_system_update();
    }

    engine_system_cleanup();
    engine_cleanup();
    return 0;
}
