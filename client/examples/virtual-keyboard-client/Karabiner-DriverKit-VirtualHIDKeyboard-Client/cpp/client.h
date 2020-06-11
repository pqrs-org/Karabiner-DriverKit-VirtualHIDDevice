#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void shared_virtual_hid_keyboard_client_initialize(void);
void shared_virtual_hid_keyboard_client_terminate(void);
int shared_virtual_hid_keyboard_client_connected(void);

void shared_virtual_hid_keyboard_client_post_control_up(void);
void shared_virtual_hid_keyboard_client_post_launchpad(void);
void shared_virtual_hid_keyboard_client_reset(void);

void shared_virtual_hid_keyboard_cilent_virtual_hid_pointing_initialize(void);

#ifdef __cplusplus
}
#endif
