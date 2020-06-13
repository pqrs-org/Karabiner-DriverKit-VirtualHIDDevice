public final class VirtualHIDDeviceClientExample {
    static let shared = VirtualHIDDeviceClientExample()

    init() {
        shared_virtual_hid_keyboard_client_initialize()
    }

    deinit {
        shared_virtual_hid_keyboard_client_terminate()
    }

    func virtualHIDKeyboardInitialize() {
        shared_virtual_hid_keyboard_initialize()
    }

    func virtualHIDKeyboardTerminate() {
        shared_virtual_hid_keyboard_terminate()
    }

    func virtualHIDKeyboardPostControlUp() {
        shared_virtual_hid_keyboard_post_control_up()
    }

    func virtualHIDKeyboardPostLaunchpad() {
        shared_virtual_hid_keyboard_post_launchpad()
    }

    func virtualHIDKeyboardReset() {
        shared_virtual_hid_keyboard_reset()
    }

    func virtualHIDPointingInitialize() {
        shared_virtual_hid_pointing_initialize()
    }

    func virtualHIDPointingTerminate() {
        shared_virtual_hid_pointing_terminate()
    }

    func virtualHIDPointingPostExampleReport() {
        shared_virtual_hid_pointing_post_example_report()
    }

    func virtualHIDPointingReset() {
        shared_virtual_hid_pointing_reset()
    }
}
