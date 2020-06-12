public final class VirtualHIDDeviceClientExample {
    static let shared = VirtualHIDDeviceClientExample()

    init() {
        shared_virtual_hid_keyboard_client_initialize()
    }

    deinit {
        shared_virtual_hid_keyboard_client_terminate()
    }

    func postControlUp() {
        shared_virtual_hid_keyboard_client_post_control_up()
    }

    func postLaunchpad() {
        shared_virtual_hid_keyboard_client_post_launchpad()
    }

    func reset() {
        shared_virtual_hid_keyboard_client_reset()
    }

    func virtualHIDPointingInitialize() {
        shared_virtual_hid_pointing_initialize()
    }

    func virtualHIDPointingPostExampleReport() {
        shared_virtual_hid_pointing_post_example_report()
    }

    func virtualHIDPointingReset() {
        shared_virtual_hid_pointing_reset()
    }
}
