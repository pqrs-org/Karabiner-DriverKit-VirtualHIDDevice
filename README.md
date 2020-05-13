# Karabiner-DriverKit-VirtualHIDDevice

Virtual devices (keyboard and mouse) implementation for macOS using DriverKit.

## Status

This project is very early stage.
There are only a stub virtual keyboard device which can be detected by macOS and an extension manager.

-   [How to be close to DriverKit](DEVELOPMENT.md)

![System Preferences](docs/images/system-preferences.png)

---

## For developers

### How to build

System requirements to build Karabiner-Elements:

-   macOS 10.15+
-   Xcode 11+
-   Command Line Tools for Xcode
-   [XcodeGen](https://github.com/yonaskolb/XcodeGen)

### Steps

1.  Replace `CODE_SIGN_IDENTITY` at `src/scripts/codesign.sh` with yours.
    (The codesign identity is required in order to inject entitlements into your driver extension even if you disabled SIP.)
2.  (Optional) Replace `G43BCU2T37` with your team identifier.
3.  (Optional) Replace `org.pqrs` with your domain.
4.  Build by the following command in terminal.

    ```shell
    cd src
    make
    ```

    `build/Release/KarabinerDriverKitVirtualHIDDevice.app` will be generated.
