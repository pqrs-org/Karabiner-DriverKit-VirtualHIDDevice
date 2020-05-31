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
2020-05-13 08:50:08.983279+0900 0x97b      Default     0x0                  0      0    kernel: (org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard) KarabinerDriverKitVirtualHIDKeyboard init
2020-05-13 08:50:08.983378+0900 0x97b      Default     0x0                  0      0    kernel: (org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard) KarabinerDriverKitVirtualHIDKeyboard handleStart
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

## How to communicate with your driver extension from user space

### Driver extension

1.  Provide your driver extension. (e.g., org_pqrs_KarabinerDriverKitVirtualHIDKeyboard)
2.  Add a subclass of IOUserClient. (e.g., org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient)
3.  Put UserClientProperties into Info.plist.

    ```xml
    <key>UserClientProperties</key>
    <dict>
        <key>IOClass</key>
        <string>IOUserUserClient</string>
        <key>IOUserClass</key>
        <string>org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient</string>
        <!-- <key>IOServiceDEXTEntitlements</key> -->
    </dict>
    ```

4.  Implement `org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::NewUserClient` method.
5.  Implement `org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::Start` and `Stop`.

    -   Save `provider` argument to ivars at `Start`.

        ```cpp
        ivars->keyboard = OSDynamicCast(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, provider);
        ```

### Client

1.  Make C++ code.

    ```cpp
      io_connect_t connect = IO_OBJECT_NULL;
      auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("org_pqrs_KarabinerDriverKitVirtualHIDKeyboard"));
      if (!service) {
        std::cerr << "IOServiceGetMatchingService error" << std::endl;
        goto finish;
      }

      {
        pqrs::osx::iokit_return ir = IOServiceOpen(service, mach_task_self(), kIOHIDServerConnectType, &connect);
        if (!ir) {
          std::cerr << "IOServiceOpen error: " << ir << std::endl;
          goto finish;
        }
      }
    ```

2.  Inject entitlements to your app.

    ```xml
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
        <dict>
            <key>com.apple.developer.driverkit.userclient-access</key>
            <array>
            <string>org.pqrs.driverkit.org_pqrs_KarabinerDriverKitVirtualHIDKeyboard</string>
            </array>
        </dict>
    </plist>
    ```

### Implement methods

You can connect to your driver extension from your client by above steps.

Implement the actual processing by the following steps.

1.  Implement `ExternalMethod` method your driverkit user client class.

    ```cpp
    kern_return_t org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::ExternalMethod(uint64_t selector,
                                                                                          IOUserClientMethodArguments* arguments,
                                                                                          const IOUserClientMethodDispatch* dispatch,
                                                                                          OSObject* target,
                                                                                          void* reference) {
        os_log(OS_LOG_DEFAULT, "ExternalMethod %llu", selector);
        return kIOReturnSuccess;
    }
    ```

2.  Call `IOConnectCallStructMethod` from your client.

    ```cpp
    IOConnectCallStructMethod(connect,
                              42,
                              nullptr, 0,
                              nullptr, 0);
    ```

    The `ExternalMethod` will be called.
