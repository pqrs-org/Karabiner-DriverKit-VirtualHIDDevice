# How to be close to DriverKit

## Doubt macOS before you suspect a problem is caused by your code

-   Restart macOS before investigating your issue.
    Replacing extension from OSSystemExtensionManager.submit does not restart your drivekit userspace process.
    The most reliable way to restart your userspace process is reboot.
-   Execute `systemextensionsctl reset` before investigating your issue.
    The reset command requires disabling SIP, however it solves various problems.

## Errors

-   `EXC_CRASH (Code Signature Invalid)`
    -   Reason:
        -   There are extra entitlements which are allowed only Apple. (e.g., `com.apple.developer.hid.virtual.device`)
-   `sysextd` is crashed by `EXC_BAD_INSTRUCTION (SIGILL)`
    -   Reason:
        -   Your driver extension is crashed in `init()` or `Start()`.
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
