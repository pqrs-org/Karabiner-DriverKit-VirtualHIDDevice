# Changelog

## Karabiner-DriverKit-VirtualHIDDevice 6.2.0

-   📅 Release date
    -   Sep 12, 2025
-   ⚡️ Improvements
    -   Updated dependent vendor code.
        -   pqrs::local_datagram v7.0

## Karabiner-DriverKit-VirtualHIDDevice 6.1.0

-   📅 Release date
    -   Aug 24, 2025
-   ⚡️ Improvements
    -   Migrated to Swift 6.

## Karabiner-DriverKit-VirtualHIDDevice 6.0.0

-   📅 Release date
    -   May 25, 2025
-   💥 Breaking changes
    -   Moved vendor directories as follows.
        If you're referencing the old paths, please update them to point to the new locations.
        -   old: `examples/virtual-hid-device-service-client/vendor/include`
        -   new: `vendor/vendor/include`
-   ⚡️ Improvements
    -   Improved the handling of termination notifications in the client for VirtualHIDDeviceRoot.
    -   Improved to reuse the existing connection as much as possible,
        rather than refreshing it on every service matched notification in the client for VirtualHIDDeviceRoot.
    -   Changed to execute `killall Karabiner-VirtualHIDDevice-Daemon` during the postinstall step.

## Karabiner-DriverKit-VirtualHIDDevice 5.0.0

-   📅 Release date
    -   Aug 30, 2024
-   💥 Breaking changes

    -   Add `vendor_id` and `product_id` into the `virtual_hid_keyboard_initialize` parameters.
        You need to modify the code as follows.

        ```diff
        - client->async_virtual_hid_keyboard_initialize(pqrs::hid::country_code::us);
        + pqrs::karabiner::driverkit::virtual_hid_device_service::virtual_hid_keyboard_parameters parameters;
        + parameters.set_country_code(pqrs::hid::country_code::us);
        + client->async_virtual_hid_keyboard_initialize(parameters);
        ```

## Karabiner-DriverKit-VirtualHIDDevice 4.3.0

-   📅 Release date
    -   May 26, 2024
-   🍰 Minor Changes
    -   Fix minor script issues.
    -   Improved `examples/SMAppServiceExample`.

## Karabiner-DriverKit-VirtualHIDDevice 4.2.0

-   📅 Release date
    -   May 20, 2024
-   🍰 Minor Changes
    -   Renamed LaunchDaemons plist name.
        -   new: `LaunchDaemons/org.pqrs.service.daemon.Karabiner-VirtualHIDDevice-Daemon.plist`
        -   old: `LaunchDaemons/org.pqrs.Karabiner-VirtualHIDDevice-Daemon.plist`

## Karabiner-DriverKit-VirtualHIDDevice 4.1.0

-   📅 Release date
    -   May 16, 2024
-   ⚡️ Improvements
    -   The signing identity during the package build has been changed from hard-coded to using an environment variable.
-   🍰 Minor Changes
    -   The provisioning profile for signing the built binary has been updated.

## Karabiner-DriverKit-VirtualHIDDevice 4.0.0

-   📅 Release date
    -   May 15, 2024
-   💥 Breaking changes
    -   macOS 11 and macOS 12 are no longer supported.
    -   The legacy `/Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist` is no longer included.
        To automatically launch the service process, please register `Karabiner-VirtualHIDDevice-Daemon.app` with launchd from your application.
        `examples/SMAppServiceExample` is an example application for performing the registration.
    -   Removed `/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/bin/cli`.
-   🔔 Compatibility
    -   There is binary compatibility for client applications. This means that a client designed for version 3.x.x will also work with version 4.0.0 without needing to rebuild your application.
-   🍰 Minor Changes
    -   The name of the daemon process has changed from `Karabiner-DriverKit-VirtualHIDDeviceClient.app` to `Karabiner-VirtualHIDDevice-Daemon.app`.

## Karabiner-DriverKit-VirtualHIDDevice 3.2.0

-   📅 Release date
    -   May 10, 2024
-   ⚡️ Improvements
    -   Changed the registration of the `Karabiner-DriverKit-VirtualHIDDeviceClient.app` in the Launch Services database to occur just once after the package is installed, instead of every time it is launched.
    -   Stopped updating the modification time of `/Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist` each time `Karabiner-DriverKit-VirtualHIDDeviceClient.app` is launched.
    -   Added `/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/bin/cli`.
    -   Updated provisionprofile files.
    -   Updated dependent vendor code.

## Karabiner-DriverKit-VirtualHIDDevice 3.1.0

-   📅 Release date
    -   Sep 17, 2023
-   💥 Breaking changes
    -   `virtual_hid_device_service::client` signals are updated:
        -   `driver_loaded_response` is changed to `driver_connected`
        -   `driver_version_matched_response` is changed to `driver_version_mismatched`
            -   Note that the true and false values are reversed.
        -   `*_response` is removed from the following signals:
            -   `virtual_hid_keyboard_ready_response`
            -   `virtual_hid_pointing_ready_response`
    -   The following obsoleted methods of `virtual_hid_device_service::client` are removed:
        -   `client(const std::filesystem::path& client_socket_file_path)`
        -   `async_driver_loaded(void)`
        -   `async_virtual_hid_keyboard_ready(void)`
        -   `async_virtual_hid_pointing_ready(void)`
-   ✨ New Features
    -   `driver_activated` signal is added into `virtual_hid_device_service::client`.
-   ⚡️ Improvements
    -   In the `forceActivate` process, if the same version of the system extension that is being installed is already installed, the request will be explicitly skipped.

## Karabiner-DriverKit-VirtualHIDDevice 2.6.0

-   📅 Release date
    -   Sep 10, 2023
-   ⚡️ Improvements
    -   Updated dependent vendor code.
        -   pqrs::osx::process_info v2.3

## Karabiner-DriverKit-VirtualHIDDevice 2.5.0

-   📅 Release date
    -   Sep 10, 2023
-   ⚡️ Improvements
    -   Fix swiftlint warnings.

## Karabiner-DriverKit-VirtualHIDDevice 2.4.0

-   📅 Release date
    -   Sep 6, 2023
-   ✨ New Features
    -   Generic desktop usage page (e.g., D-pad) has been supported.

## Karabiner-DriverKit-VirtualHIDDevice 2.3.0

-   📅 Release date
    -   Aug 19, 2023
-   ⚡️ Improvements
    -   Updated dependent vendor code.
        -   asio 1.28.1

## Karabiner-DriverKit-VirtualHIDDevice 2.2.0

-   📅 Release date
    -   Jul 17, 2023
-   ⚡️ Improvements
    -   Updated dependent vendor code.
        -   asio 1.28.0
        -   spdlog 1.12.0

## Karabiner-DriverKit-VirtualHIDDevice 2.1.0

-   📅 Release date
    -   Apr 15, 2023
-   🐛 Bug Fixes
    -   Fixed an issue that the virtual keyboard was not recreated with the new country code when the country code was changed after the virtual keyboard was initialized.

## Karabiner-DriverKit-VirtualHIDDevice 2.0.0

-   📅 Release date
    -   Jan 5, 2023
-   💥 Breaking changes
    -   The following callback in virtual_hid_device_service::client are now called periodically without a request.
        -   driver_loaded_response
        -   driver_version_matched_response
        -   virtual_hid_keyboard_ready_response
        -   virtual_hid_pointing_ready_response
-   ⚡️ Improvements
    -   The virtual device regeneration is no longer performed even when virtual_hid_keyboard_initialize or virtual_hid_pointing_initialize calls are repeated within a short period of time.

## Karabiner-DriverKit-VirtualHIDDevice 1.35.0

-   📅 Release date
    -   Jan 3, 2023
-   ⚡️ Improvements
    -   Fix log messages.
    -   Update dependent vendor code.

## Karabiner-DriverKit-VirtualHIDDevice 1.34.0

-   📅 Release date
    -   Jan 2, 2023
-   ⚡️ Improvements
    -   Improved error recovery by refreshing the client socket path for each connection.
    -   The keyboard and pointing device now share the same socket for communication.
    -   Update dependent vendor code.

## Karabiner-DriverKit-VirtualHIDDevice 1.33.0

-   📅 Release date
    -   Dec 29, 2022
-   ⚡️ Improvements
    -   Update dependent vendor code

## Karabiner-DriverKit-VirtualHIDDevice 1.32.0

-   📅 Release date
    -   Dec 28, 2022
-   ⚡️ Improvements
    -   Update dependent vendor code

## Karabiner-DriverKit-VirtualHIDDevice 1.31.0

-   📅 Release date
    -   Dec 27, 2022
-   ⚡️ Improvements
    -   Added `AssociatedBundleIdentifiers` into `LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist`
    -   Improved error recovery by enhanced status check of local datagram communication between virtual_hid_device_service_server and virtual_hid_device_service::client.

## Karabiner-DriverKit-VirtualHIDDevice 1.30.0

-   📅 Release date
    -   Jun 5, 2022
-   ⚡️ Improvements
    -   Ignore `async_virtual_hid_keyboard_initialize` and `async_virtual_hid_pointing_initialize` if the device is already initialized.

## Karabiner-DriverKit-VirtualHIDDevice 1.29.0

-   📅 Release date
    -   May 6, 2022
-   ⚡️ Improvements
    -   The duplicated `driver_version_ is mismatched` warning log messages have suppressed.

## Karabiner-DriverKit-VirtualHIDDevice 1.28.0

-   📅 Release date
    -   May 5, 2022
-   ⚡️ Improvements
    -   Remove dependency of deprecated `kIOMasterPortDefault`.

## Karabiner-DriverKit-VirtualHIDDevice 1.27.0

-   📅 Release date
    -   Nov 19, 2021
-   ⚡️ Improvements
    -   Reverted the virtual keyboard usage maximum to 255 to avoid ioreg CPU usage issue on macOS 12 Monterey.
        -   Changed the consumer usage maximum to 768.
    -   Updated driver version to 1.6.0.

## Karabiner-DriverKit-VirtualHIDDevice 1.26.0

-   📅 Release date
    -   Oct 30, 2021
-   ⚡️ Improvements
    -   Reverted to build with Xcode 13.0 due to Xcode 13.1 generates dext which does not support macOS 11 Big Sur.
    -   Updated driver version to 1.5.0 to ignore dext in v1.25.0.

## Karabiner-DriverKit-VirtualHIDDevice 1.25.0

-   📅 Release date
    -   Oct 30, 2021
-   ⚡️ Improvements
    -   Built with Xcode 13.1)

## Karabiner-DriverKit-VirtualHIDDevice 1.24.0

-   📅 Release date
    -   Oct 20, 2021
-   ⚡️ Improvements
    -   Changed the virtual keyboard usage maximum to 65535 from 255.
    -   Changed to create virtual devices for each pqrs::karabiner::driverkit::virtual_hid_device_service::client.
    -   Updated driver version to 1.4.0.

## Karabiner-DriverKit-VirtualHIDDevice 1.23.0

-   📅 Release date
    -   Sep 25, 2021
-   ⚡️ Improvements
    -   Updated icons of `/Applications/.Karabiner-VirtualHIDDevice-Manager.app`. (Thanks to Kouji TAMURA)
