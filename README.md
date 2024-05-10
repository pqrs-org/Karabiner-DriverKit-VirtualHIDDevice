[![Build Status](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/workflows/CI/badge.svg)](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/actions)
[![License](https://img.shields.io/badge/license-Public%20Domain-blue.svg)](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/blob/main/LICENSE.md)

# Karabiner-DriverKit-VirtualHIDDevice

Virtual devices (keyboard and mouse) implementation for macOS using DriverKit.

## Supported systems

-   macOS 14 Sonoma
    -   Both Intel-based Macs and Apple Silicon Macs
-   macOS 13 Ventura
    -   Both Intel-based Macs and Apple Silicon Macs
-   macOS 12 Monterey
    -   Both Intel-based Macs and Apple Silicon Macs
-   macOS 11 Big Sur
    -   Both Intel-based Macs and Apple Silicon Macs

## Status

-   Implemented:
    -   Extension manager
    -   Virtual HID keyboard
    -   Virtual HID pointing
    -   Virtual HID device client

## Documents

-   [How to be close to DriverKit](DEVELOPMENT.md)
-   [Extracts from xnu](XNU.md)

## Screenshots

-   macOS Settings (macOS detects the virtual keyboard)<br/><br />
    <img src="docs/images/macos-settings@2x.png" width="668" alt="System Preferences" /><br /><br />

---

## Usage

1.  Open `dist/Karabiner-DriverKit-VirtualHIDDevice-x.x.x.pkg`.
2.  Install files via installer.
3.  Execute the following command in Terminal.

    ```shell
    /Applications/.Karabiner-VirtualHIDDevice-Manager.app/Contents/MacOS/Karabiner-VirtualHIDDevice-Manager activate
    ```

4.  Run a client program to test the driver extension.

    ```shell
    git clone --depth 1 https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice.git
    cd Karabiner-DriverKit-VirtualHIDDevice/examples/virtual-hid-device-service-client
    brew install xcodegen
    make
    make run
    ```

## Uninstallation

1.  Run uninstaller in Terminal.

    ```shell
    bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall/deactivate_driver.sh'
    sudo bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall/remove_files.sh'
    ```

### Installed files

-   `/Applications/.Karabiner-VirtualHIDDevice-Manager.app`
-   `/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice`
-   `/Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist`
-   `/Library/Application Support/org.pqrs/tmp`
-   `/var/log/karabiner`

---

## For developers

### How to build

System requirements to build Karabiner-Elements:

-   macOS 11+
-   Xcode 13.0 (You need to hold Xcode version to 13.0 because Xcode 13.1 generate binary which does not work on macOS 11 Big Sur.)
    -   Note: The recent macOS cannot open the Xcode 13.0 UI, but it can be operated from the command line. Since only command line operations are necessary for building, there is no problem.
-   Command Line Tools for Xcode
-   [XcodeGen](https://github.com/yonaskolb/XcodeGen)

### Steps

1.  Obtain DriverKit entitlements to create a provisioning profile that supports `com.apple.developer.driverkit`.
    Specifically, follow the instructions on [Requesting Entitlements for DriverKit Development](https://developer.apple.com/documentation/driverkit/requesting_entitlements_for_driverkit_development)

    Note: This process may take some time to be completed on Apple's side.

2.  Create App IDs on [the Apple Developer site](https://developer.apple.com/account/resources/identifiers/list).

<table>
    <thead>
        <tr>
            <th>Bundle ID</th>
            <th>Capabilities</th>
            <th>App Services</th>
            <th>Additional Capabilities</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>org.pqrs.Karabiner-DriverKit-VirtualHIDDevice</td>
            <td>---</td>
            <td>---</td>
            <td>
                com.apple.developer.driverkit<br/>
                com.apple.developer.driverkit.family.hid.device<br/>
                com.apple.developer.driverkit.family.hid.eventservice<br/>
                com.apple.developer.driverkit.transport.hid<br/>
                com.apple.developer.hid.virtual.device<br/>
            </td>
        </tr>
        <tr>
            <td>org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient</td>
            <td>---</td>
            <td>---</td>
            <td>---</td>
        </tr>
        <tr>
            <td>org.pqrs.Karabiner-VirtualHIDDevice-Manager</td>
            <td>
                System Extension<br/>
            </td>
            <td>---</td>
            <td>---</td>
        </tr>
    </tbody>
</table>

<img src="docs/images/additional-capabilities@2x.png" width="921" alt="Additional Capabilities" />

3.  Create a profile corresponding to the App IDs on the Apple Developer site.

4.  Replace the `*.provisionprofile` files in the repository with your own provision profile files.

    -   src/Client/Developer_ID_VirtualHIDDeviceClient.provisionprofile
    -   src/DriverKit/Developer_ID_KarabinerDriverKitVirtualHIDDevice.provisionprofile
    -   src/Manager/Developer_ID_Karabiner_VirtualHIDDevice_Manager.provisionprofile

5.  Replace `CODE_SIGN_IDENTITY` at `src/scripts/codesign.sh` with yours.

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

    Choose one of them (e.g., `8D660191481C98F5C56630847A6C39D95C166F22`) and replace existing `CODE_SIGN_IDENTITY` with yours as follows.

    ```shell
    # Replace with your identity
    readonly CODE_SIGN_IDENTITY=8D660191481C98F5C56630847A6C39D95C166F22
    ```

6.  Replace team identifier, domain and embedded.provisionprofile.

    -   Search `G43BCU2T37` and replace them with your team identifier.

        ```shell
        git grep G43BCU2T37 src/
        ```

    -   Search `org.pqrs` and `org_pqrs`, then replace them with your domain.

        ```shell
        git grep org.pqrs src/
        git grep org_pqrs src/
        ```

    -   Replace `embedded.provisionprofile` file with yours.

        ```shell
        find * -name 'embedded.provisionprofile'
        ```

7.  Build by the following command in terminal.

    ```shell
    make package
    ```

    `dist/Karabiner-DriverKit-VirtualHIDDevice-X.X.X.pkg` will be generated.

### Components

Karabiner-DriverKit-VirtualHIDDevice consists the following components.

-   Extension Manager (including DriverKit driver)
    -   `/Applications/.Karabiner-VirtualHIDDevice-Manager.app`
    -   It provides a command line interface to activate or deactivate DriverKit driver.
-   VirtualHIDDeviceClient
    -   `/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/Applications/Karabiner-DriverKit-VirtualHIDDeviceClient.app`
    -   It mediates between the client app and the driver.
    -   It allows apps to communicate with the virtual device even if the app is not signed with pqrs.org's code signing identity.
        (The client app must be running with root privileges.)
-   Client apps
    -   Client apps are not included in the distributed package.
    -   For example, you can build the client app from `examples/virtual-hid-device-service-client` in this repository.
    -   Client apps can send input events by communicating with VirtualHIDDeviceClient via UNIX domain socket.
        (`/Library/Application Support/org.pqrs/tmp/rootonly/vhidd_server/*.sock`)

![components.svg](./docs/plantuml/output/components.svg)

### Version files

-   `version`:
    -   Karabiner-DriverKit-VirtualHIDDevice package version.
    -   Increment when any components are updated.
-   `driver-version`:
    -   DriverKit driver internal version.
    -   Increment when the driver source code is updated.
