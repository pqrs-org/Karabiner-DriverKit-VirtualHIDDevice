# Changelog

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
