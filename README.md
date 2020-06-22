[![Build Status](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/workflows/CI/badge.svg)](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/actions)
[![License](https://img.shields.io/badge/license-Public%20Domain-blue.svg)](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/blob/master/LICENSE.md)

# Karabiner-DriverKit-VirtualHIDDevice

Virtual devices (keyboard and mouse) implementation for macOS using DriverKit.

## Status

-   Implemented:
    -   Extension manager
    -   Virtual HID keyboard
    -   Virtual HID pointing
    -   Virtual HID device client (Need to disable SIP at the moment.)

## Documents

-   [How to be close to DriverKit](DEVELOPMENT.md)
-   [Extracts from xnu](XNU.md)

## Screenshots

-   System Preferenecs (macOS detects the virtual keyboard)<br/><br />
    <img src="docs/images/system-preferences@2x.png" width="668" alt="System Preferences" /><br /><br />
-   Extension manager<br/><br />
    <img src="docs/images/extension-manager@2x.png" width="798" alt="Extension manager" /><br /><br />
-   Client<br/><br />
    <img src="docs/images/client@2x.png" width="480" alt="Client" /><br /><br />

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

    Find your codesign identity by executing the following command in Terminal.

    ```shell
    security find-identity -p codesigning -v
    ```

    The result is as follows.

    ```text
    1) 8D660191481C98F5C56630847A6C39D95C166F22 "Developer ID Application: Fumihiko Takayama (G43BCU2T37)"
    2) 6B9AF0D3B3147A69C5E713773ADD9707CB3480D9 "Apple Development: Fumihiko Takayama (YVB3SM6ECS)"
    3) 637B86ED1C06AE99854E9F5A5DCE02DA58F2BBF4 "Mac Developer: Fumihiko Takayama (YVB3SM6ECS)"
    4) 987BC26C6474DF0C0AF8BEA797354873EC83DC96 "Apple Distribution: Fumihiko Takayama (G43BCU2T37)"
        4 valid identities found
    ```

    Choose one of them (e.g., `6B9AF0D3B3147A69C5E713773ADD9707CB3480D9`) and replace existing `CODE_SIGN_IDENTITY` with yours as follows.

    ```shell
    # Replace with your identity
    readonly CODE_SIGN_IDENTITY=6B9AF0D3B3147A69C5E713773ADD9707CB3480D9
    ```

2.  (Optional) Replace team identifier, domain and embedded.provisionprofile if you want to test your driver with SIP enabled environments.

    -   Search `G43BCU2T37` and replace them with your team identifier if you want to test your driver with SIP enabled environments.

        ```shell
        git grep G43BCU2T37 src/
        ```

    -   Search `org.pqrs` and `org_pqrs`, then replace them with your domain if you want to test your driver with SIP enabled environments.

        ```shell
        git grep org.pqrs src/
        git grep org_pqrs src/
        ```

    -   Replace `embedded.provisionprofile` file with yours.

        ```shell
        find * -name 'embedded.provisionprofile'
        ```

3.  Build by the following command in terminal.

    ```shell
    cd src
    make
    ```

    `build/Release/KarabinerDriverKitVirtualHIDDevice.app` will be generated.
