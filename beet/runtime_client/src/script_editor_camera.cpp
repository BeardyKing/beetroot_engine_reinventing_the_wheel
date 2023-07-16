#include <client/script_editor_camera.h>

#include <gfx/gfx_resource_db.h>

#include <core/input.h>
#include <core/window.h>
#include <core/time.h>

#include <math/quat.h>

void script_update_editor_camera() {
    const CameraEntity *camEntity = gfx_db_get_camera_entity(0);
    Transform *transform = gfx_db_get_transform(camEntity->transformIndex);

    if (input_mouse_pressed(MouseButton::Right)) {
        window_set_cursor(CursorState::HiddenLockedLockMousePos);
        window_set_cursor_lock_position(input_mouse_position());
    }

    if (input_mouse_released(MouseButton::Right)) {
        window_set_cursor(CursorState::Normal);
    }

    if (input_mouse_down(MouseButton::Right)) {
        const vec2f delta = input_mouse_delta();
        const float mouseSpeed = 30.0f;
        transform->rotation.y += (-delta.x * (float) time_delta()) * mouseSpeed;
        transform->rotation.x += (-delta.y * (float) time_delta()) * mouseSpeed;

        vec3f moveDirection{};
        const vec3f camForward = quat(transform->rotation) * WORLD_FORWARD;
        const vec3f camRight = quat(transform->rotation) * WORLD_RIGHT;

        float moveSpeed = 8.0f;
        const float moveSpeedScalar = 4.0f;
        if (input_key_down(KeyCode::W)) {
            moveDirection += camForward;
        }
        if (input_key_down(KeyCode::S)) {
            moveDirection += -camForward;
        }
        if (input_key_down(KeyCode::A)) {
            moveDirection += camRight;
        }
        if (input_key_down(KeyCode::D)) {
            moveDirection += -camRight;
        }
        if (input_key_down(KeyCode::R)) {
            moveDirection += WORLD_UP;
        }
        if (input_key_down(KeyCode::F)) {
            moveDirection += -WORLD_UP;
        }
        if (input_key_down(KeyCode::Shift)) {
            moveSpeed *= moveSpeedScalar;
        }
        transform->position += moveDirection * ((float) time_delta() * moveSpeed);
    }
}