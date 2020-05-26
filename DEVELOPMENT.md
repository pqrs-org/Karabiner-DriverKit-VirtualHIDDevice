# How to be close to DriverKit

## Doubt macOS before you suspect a problem is caused by your code

-   Restart macOS before investigating your issue.
    Replacing extension from `OSSystemExtensionManager.submit` does not restart your driverkit userspace process.
    The most reliable way to restart your userspace process is reboot.
-   Execute `systemextensionsctl reset` before investigating your issue.
    The reset command requires disabling SIP, however it solves various problems.

## Most reliable way to upgrade your driver extension

1.  Execute `systemextensionsctl reset`.
2.  Install your driver extension from ExtensionManager. (User approval is always required due to `systemextensionsctl reset`.)
3.  Restart your macOS.

## Log messages

Using `log` command to show your driver log messages.

```shell
log show --predicate 'sender == "sysextd" or sender CONTAINS "org.pqrs"' --info --debug --last 1h
```

The result:

```text
Timestamp                       Thread     Type        Activity             PID    TTL
2020-05-13 08:50:08.983279+0900 0x97b      Default     0x0                  0      0    kernel: (org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard) org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard init
2020-05-13 08:50:08.983378+0900 0x97b      Default     0x0                  0      0    kernel: (org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard) org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard handleStart
2020-05-13 08:50:08.983521+0900 0x97b      Default     0x0                  0      0    kernel: (org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard) [IOUserHIDDevice.cpp:62][0x100000514] Start failed: 0xe00002c7
```

## Inspect installed driver extensions

See `db.plist`.

```shell
plutil -convert xml1 -o - /Library/SystemExtensions/db.plist
```

## Errors

-   `EXC_CRASH (Code Signature Invalid)`
    -   Reason:
        -   There are extra entitlements which are not allowed for us:
            -   `com.apple.developer.hid.virtual.device`
            -   `com.apple.developer.system-extension.redistributable` (Bug?)
-   `sysextd` is crashed by `EXC_BAD_INSTRUCTION (SIGILL)`
    -   Reason #1:
        -   `sysextd` will be crashed if multiple versions of your driver extension are installed.
        -   Workaround:
            -   `systemextensionsctl reset`
    -   Reason #2:
        -   Your driver extension is crashed in `init()` or `Start()`.
            Add log messages to investigate the problem.
-   `sysextd: (libswiftCore.dylib) Fatal error: Activate found 2 extensions in active state, ID: xxx`
    -   Workaround:
        -   Execute `systemextensionsctl reset`, then install your system extension again.

## Build issues

-   Error: Xcode requires a provisioning profile which supports DriverKit
    -   Reason:
        -   The driverkit entitlements (e.g., `com.apple.developer.driverkit`) requires a proper provisioning profile which you cannot create it unless you gained DriverKit framework capability from Apple.
    -   Workaround:
        -   If you want to develop driver extension without the capability, build your code without entitlements and inject entitlements at codesigning stage.
            See `src/scripts/codesign.sh` for details.

---

## How to communicate with your driver extension from user space (WIP)

1.  Provide your driver extension. (e.g., org_pqrs_KarabinerDriverKitVirtualHIDKeyboard)
2.  Add a subclass of IOUserClient. (e.g., org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient)
3.  Put UserClientProperties into Info.plist.

    ```xml
    <key>UserClientProperties</key>
    <dict>
        <key>IOClass</key>
        <string>IOUserUserClient</string>
        <key>IOServiceDEXTEntitlements</key>
        <array>
        <string>com.apple.developer.driverkit</string>
        <string>com.apple.developer.driverkit.family.hid.device</string>
        <string>com.apple.developer.driverkit.family.hid.eventservice</string>
        <string>com.apple.developer.driverkit.family.hid.virtual.device</string>
        <string>com.apple.developer.driverkit.transport.hid</string>
        </array>
        <key>IOUserClass</key>
        <string>org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient</string>
    </dict>
    ```

4.  Implement `org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::NewUserClient` method.
5.  Implement `org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::Start` and `Stop`.
    Save `provider` argument to ivars at `Start`.

    ```cpp
    ivars->keyboard = OSDynamicCast(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, provider);
    ```
