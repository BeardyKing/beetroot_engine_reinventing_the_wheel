#include <beet/input.h>

//===internal structs========
struct KeyInfo {
    bool keyDown;
    float timePressed;
    float timeReleased;
};

struct Input {
    KeyInfo keys[(size_t) KeyCode::Last]{};
    float cachedCurrentTime{};
};

Input *g_input;

//===internal functions======
//===api=====================
bool input_key_pressed(KeyCode key) {
    KeyInfo *keyInfo = &g_input->keys[(size_t) key];
    return keyInfo->timePressed == g_input->cachedCurrentTime;
}

bool input_key_down(KeyCode key) {
    return g_input->keys[(size_t) key].keyDown;
}

void input_set_time(double time) {
    g_input->cachedCurrentTime = (float) time;
}

float input_key_down_time(KeyCode key) {
    KeyInfo *keyInfo = &g_input->keys[(size_t) key];
    if (keyInfo->timePressed > keyInfo->timeReleased) {
        return g_input->cachedCurrentTime - keyInfo->timePressed;
    }
    return 0.0f;
}

//===init & shutdown=========
void input_create() {
    g_input = new Input{};
}

void input_cleanup() {
    delete g_input;
    g_input = nullptr;
}

void input_key_down_callback(int32_t keyCode) {
    KeyInfo *keyInfo = &g_input->keys[keyCode];
    if (!keyInfo->keyDown) {
        keyInfo->keyDown = true;
        keyInfo->timePressed = g_input->cachedCurrentTime;
    }
}

void input_key_up_callback(const int32_t keyCode) {
    KeyInfo *keyInfo = &g_input->keys[keyCode];
    keyInfo->keyDown = false;
    keyInfo->timeReleased = g_input->cachedCurrentTime;
}