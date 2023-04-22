#ifndef VK_BEETROOT_INPUT_H
#define VK_BEETROOT_INPUT_H

#include <beet/types.h>

//===api=====================
void input_set_time(double time);
bool input_key_pressed(KeyCode key);
bool input_key_down(KeyCode key);
float input_key_down_time(KeyCode key);

//===init & shutdown=========
void input_create();
void input_cleanup();

void input_key_down_callback(int32_t asciiKeyCode);
void input_key_up_callback(int32_t asciiKeyCode);

#endif //VK_BEETROOT_INPUT_H
