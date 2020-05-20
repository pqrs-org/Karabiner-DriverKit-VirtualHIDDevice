# Karabiner-DriverKit-VirtualHIDDevice

Virtual devices (keyboard and mouse) implementation for macOS using DriverKit.

## Status

This project is very early stage.
There are only a stub virtual keyboard device which can be detected by macOS, and an extension manager.

-   [How to be close to DriverKit](DEVELOPMENT.md)

![System Preferences](docs/images/system-preferences.png)

![Extension Manager](docs/images/extension-manager.png)

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
    (The codesign identity is required even if you disabled SIP in order to inject entitlements into your driver extension.)
2.  (Optional) Search `G43BCU2T37` and replace them with your team identifier if you want to test your driver with SIP enabled environments.

    ```shell
    git grep G43BCU2T37 src/
    ```

3.  (Optional) Search `org.pqrs` and replace them with your domain if you want to test your driver with SIP enabled environments.

    ```shell
    git grep org.pqrs src/
    ```

4.  Build by the following command in terminal.

    ```shell
    cd src
    make
    ```

    `build/Release/KarabinerDriverKitVirtualHIDDevice.app` will be generated.
