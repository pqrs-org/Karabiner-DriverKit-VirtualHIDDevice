import IOKit.hid
import SwiftUI

private func callback(context: UnsafeMutableRawPointer?,
                      result _: IOReturn,
                      sender _: UnsafeMutableRawPointer?,
                      device: IOHIDDevice?)
{
    let obj: DeviceManager! = unsafeBitCast(context, to: DeviceManager.self)
    obj.virtualHIDKeyboard = device
}

class DeviceManager: ObservableObject {
    static let shared = DeviceManager()

    let hidManager: IOHIDManager
    @Published var virtualHIDKeyboard: IOHIDDevice?

    init() {
        hidManager = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone))

        let matching = IOServiceNameMatching("org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard")
        // let matching = IOServiceMatching(kIOHIDDeviceKey)
        IOHIDManagerSetDeviceMatching(hidManager, matching)

        IOHIDManagerRegisterDeviceMatchingCallback(hidManager, callback, unsafeBitCast(self, to: UnsafeMutableRawPointer.self))

        IOHIDManagerScheduleWithRunLoop(hidManager,
                                        CFRunLoopGetMain(),
                                        CFRunLoopMode.defaultMode!.rawValue)

        IOHIDManagerOpen(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))
    }

    func setHIDValue(_ integerValue: Int) {
        if virtualHIDKeyboard == nil {
            return
        }

        if let elements = IOHIDDeviceCopyMatchingElements(virtualHIDKeyboard!,
                                                          nil,
                                                          IOOptionBits(kIOHIDOptionsTypeNone)) as NSArray? as? [IOHIDElement]
        {
            for element in elements {
                if IOHIDElementGetUsagePage(element) == kHIDPage_LEDs,
                    IOHIDElementGetUsage(element) == kHIDUsage_LED_CapsLock,
                    IOHIDElementGetType(element) == kIOHIDElementTypeOutput
                {
                    //
                    // Value
                    //

                    let value = IOHIDValueCreateWithIntegerValue(kCFAllocatorDefault,
                                                                 element,
                                                                 mach_absolute_time(),
                                                                 integerValue)
                    print("setHIDValue \(integerValue)")
                    print(element)
                    IOHIDDeviceSetValueWithCallback(virtualHIDKeyboard!, element, value, 0.1, { _, _, _, _ in
                        print("IOHIDDeviceSetValue callback")
                    }, nil)
                }
            }
        }
    }

    func setInvalidHIDReport() {
        if virtualHIDKeyboard == nil {
            return
        }

        if let elements = IOHIDDeviceCopyMatchingElements(virtualHIDKeyboard!,
                                                          nil,
                                                          IOOptionBits(kIOHIDOptionsTypeNone)) as NSArray? as? [IOHIDElement]
        {
            for element in elements {
                if IOHIDElementGetUsagePage(element) == kHIDPage_LEDs,
                    IOHIDElementGetUsage(element) == kHIDUsage_LED_CapsLock,
                    IOHIDElementGetType(element) == kIOHIDElementTypeOutput
                {
                    //
                    // Report
                    //

                    IOHIDDeviceSetReportWithCallback(virtualHIDKeyboard!, kIOHIDReportTypeOutput, 5, [], 0, 0.1, { _, _, _, _, _, _, _ in
                        print("IOHIDDeviceSetReport callback")
                    }, nil)
                }
            }
        }
    }
}
