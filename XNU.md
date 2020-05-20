# Extracts from xnu

-   gIODriverKitUserClientEntitlementsKey == "com.apple.developer.driverkit.userclient-access"
-   gIOModuleIdentifierKey == "CFBundleIdentifier"
-   gIOServiceDEXTEntitlementsKey == "IOServiceDEXTEntitlements"

---

## gIODriverKitUserClientEntitlementsKey

-   xnu-6153.61.1/iokit/Kernel/IOService.cpp

    ```cpp
    gIODriverKitUserClientEntitlementsKey   = OSSymbol::withCStringNoCopy( kIODriverKitUserClientEntitlementsKey );
    ```

-   xnu-6153.61.1/iokit/IOKit/IOKitKeys.h

    ```cpp
    #define kIODriverKitUserClientEntitlementsKey "com.apple.developer.driverkit.userclient-access"
    ```

## gIOModuleIdentifierKey

-   xnu-6153.61.1/iokit/Kernel/IOCatalogue.cpp

    ```cpp
    gIOModuleIdentifierKey       = OSSymbol::withCStringNoCopy( kCFBundleIdentifierKey );
    ```

-   xnu-6153.61.1/libkern/libkern/OSKextLib.h

    ```cpp
    #define kCFBundleIdentifierKey                  "CFBundleIdentifier"
    ```

## gIOServiceDEXTEntitlementsKey

-   xnu-6153.61.1/iokit/Kernel/IOService.cpp

    ```cpp
    gIOServiceDEXTEntitlementsKey           = OSSymbol::withCStringNoCopy( kIOServiceDEXTEntitlementsKey );
    ```

-   xnu-6153.61.1/iokit/IOKit/IOKitKeys.h

    ```cpp
    #define kIOServiceDEXTEntitlementsKey   "IOServiceDEXTEntitlements"
    ```
