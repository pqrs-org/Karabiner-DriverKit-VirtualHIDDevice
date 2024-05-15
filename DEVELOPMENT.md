# How to be close to DriverKit

## Execute `systemextensionsctl reset` and reboot macOS before you suspect a problem is caused by your code

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
2020-07-26 23:43:10.674449+0900 0x973      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceRoot 0.12.0 init
2020-07-26 23:43:10.674525+0900 0x973      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceRoot 0.12.0 Start
2020-07-26 23:44:28.781849+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceRoot 0.12.0 NewUserClient type:0
2020-07-26 23:44:28.781895+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceUserClient 0.12.0 init
2020-07-26 23:44:28.781940+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceUserClient 0.12.0 Start
2020-07-26 23:44:28.781943+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDDeviceRoot 0.12.0 UserClient is created
2020-07-26 23:44:28.782094+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDKeyboard 0.12.0 init
2020-07-26 23:44:28.782153+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDKeyboard 0.12.0 handleStart
2020-07-26 23:44:28.782238+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDKeyboard 0.12.0 newDeviceDescription
2020-07-26 23:44:28.782264+0900 0x974      Default     0x0                  0      0    kernel: (org.pqrs.Karabiner-DriverKit-VirtualHIDDevice) Karabiner-DriverKit-VirtualHIDKeyboard 0.12.0 newReportDescriptor
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
        -   Your `dext` must be notarized if you enabled SIP even if the dext is built on the install target machine.
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

## OSSystemExtensionError.Code

| Code | Name                            | Description                                                                                                                                |
| ---- | ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| 1    | unknown                         | An error code that indicates an unknown error occurred.                                                                                    |
| 2    | missingEntitlement              | An error code that indicates the system extension lacks a required entitlement.                                                            |
| 3    | unsupportedParentBundleLocation | An error code that indicates the extension’s parent app isn’t in a valid location for activation.                                          |
| 4    | extensionNotFound               | An error code that indicates the manager can’t find the system extension.                                                                  |
| 5    | extensionMissingIdentifier      | An error code that indicates the extension identifier is missing.                                                                          |
| 6    | duplicateExtensionIdentifer     | An error code that indicates the extension identifier duplicates an existing identifier.                                                   |
| 7    | unknownExtensionCategory        | An error code that indicates the extension manager can’t recognize the extension’s category identifier.                                    |
| 8    | codeSignatureInvalid            | An error code that indicates the extension’s signature is invalid.                                                                         |
| 9    | validationFailed                | An error code that indicates the manager can’t validate the extension.                                                                     |
| 10   | forbiddenBySystemPolicy         | An error code that indicates the system policy prohibits activating the system extension.                                                  |
| 11   | requestCanceled                 | ExtensionsAn error code that indicates the system extension manager request was canceled.                                                  |
| 12   | requestSuperseded               | An error code that indicates the system extension request failed because the system already has a pending request for the same identifier. |
| 13   | authorizationRequired           | An error code that indicates the system was unable to obtain the proper authorization.                                                     |

---

## How to register IOUserHIDDevice

We should not override `Start` method in the subclass of `IOUserHIDDevice`.

[IOUserHIDDevice::Start documentation](https://developer.apple.com/documentation/hiddriverkit/iouserhiddevice/3433772-start)

So, we cannot call `IOService::RegisterService` directly at the end of `Start` method.
And if you call `RegisterService` at the end of `handleStart`, your own device will not be matched because the initialization process has not been completed at `RegisterService` is called.

Thus, the correct way to register `IOUserHIDDevice` is that we implement `newDeviceDescription` and return `"RegisterService"`.

```cpp
OSDictionary* org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::newDeviceDescription(void) {
  auto dictionary = OSDictionary::withCapacity(12);

  ...

  OSDictionarySetValue(dictionary, "RegisterService", kOSBooleanTrue);

  ...

  return dictionary;
}
```

The `RegisterService` invokes `registerService()` at [IOHIDDevice::start](https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/blob/main/docs/vendor/IOHIDFamily/IOHIDDevice.cpp#L476-L479).

---

## How to communicate with your driver extension from user space

### Driver extension

1.  Provide your driver extension. (e.g., org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot)
2.  Add a subclass of IOUserClient. (e.g., org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient)
3.  Put UserClientProperties into Info.plist.

    ```xml
    <key>UserClientProperties</key>
    <dict>
        <key>IOClass</key>
        <string>IOUserUserClient</string>
        <key>IOUserClass</key>
        <string>org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient</string>
        <!-- <key>IOServiceDEXTEntitlements</key> -->
    </dict>
    ```

4.  Implement `org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot::NewUserClient` method.
5.  Implement `org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::Start` and `Stop`.

    -   You can save `provider` argument to ivars at `org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::Start` if needed as follows.

        ```cpp
        ivars->deviceRoot = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot, provider);
        ```

### Client

1.  Make C++ code.

    ```cpp
      io_connect_t connect = IO_OBJECT_NULL;
      auto service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceRoot"));
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
            <string>org.pqrs.Karabiner-DriverKit-VirtualHIDDevice</string>
            </array>
        </dict>
    </plist>
    ```

### Implement methods

You can connect to your driver extension from your client by above steps.

Implement the actual processing by the following steps.

1.  Implement `ExternalMethod` method your driverkit user client class.

    ```cpp
    kern_return_t org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::ExternalMethod(uint64_t selector,
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
