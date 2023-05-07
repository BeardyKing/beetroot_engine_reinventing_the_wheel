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
        window_create_render_surface(gfx_instance(), gfx_surface());
    });
    engine_register_system_create(4, socket_example_create_client);

    engine_register_system_update(0, time_tick);
    engine_register_system_update(1, []() { input_set_time(time_current()); });
    engine_register_system_update(2, window_update);
    engine_register_system_update(3, input_update);
//    engine_register_system_update(4, socket_example_update_client);

    //executed as reverse iter
    engine_register_system_cleanup(0, window_cleanup);
    engine_register_system_cleanup(1, time_cleanup);
    engine_register_system_cleanup(2, input_cleanup);
    engine_register_system_cleanup(3, gfx_cleanup);
    engine_register_system_cleanup(4, socket_example_cleanup_client);
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
