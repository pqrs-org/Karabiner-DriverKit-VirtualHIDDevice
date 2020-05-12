#include <DriverKit/IOLib.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/OSCollections.h>
#include <HIDDriverKit/IOHIDDeviceKeys.h>
#include <os/log.h>

#include "KarabinerDriverKitVirtualHIDKeyboard.h"

namespace {
const uint8_t reportDescriptor_[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report Id (1)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x19, 0xe0,       //   Usage Minimum........... (224)
    0x29, 0xe7,       //   Usage Maximum........... (231)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x25, 0x01,       //   Logical Maximum......... (1)
    0x75, 0x01,       //   Report Size............. (1)
    0x95, 0x08,       //   Report Count............ (8)
    0x81, 0x02,       //   Input...................(Data, Variable, Absolute)
                      //
    0x95, 0x01,       //   Report Count............ (1)
    0x75, 0x08,       //   Report Size............. (8)
    0x81, 0x01,       //   Input...................(Constant)
                      //
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x08,       //   Report Size............. (8)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x29, 0xff,       //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x05, 0x0c,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage 1 (kHIDUsage_Csmr_ConsumerControl)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x02,       //   Report Id (2)
    0x05, 0x0c,       //   Usage Page (Consumer)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x08,       //   Report Size............. (8)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x29, 0xff,       //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x06, 0x00, 0xff, // Usage Page (kHIDPage_AppleVendor)
    0x09, 0x01,       // Usage 1 (kHIDUsage_AppleVendor_TopCase)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x03,       //   Report Id (3)
    0x05, 0xff,       //   Usage Page (kHIDPage_AppleVendorTopCase)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x08,       //   Report Size............. (8)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x29, 0xff,       //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x06, 0x00, 0xff, // Usage Page (kHIDPage_AppleVendor)
    0x09, 0x06,       // Usage 6 (kHIDUsage_AppleVendor_Keyboard)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x04,       //   Report Id (4)
    0x06, 0x01, 0xff, //   Usage Page (kHIDPage_AppleVendorKeyboard)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x08,       //   Report Size............. (8)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x29, 0xff,       //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xa1, 0x01, // Collection (Application)
    0x85, 0x05, //   Report Id (5)
    0x05, 0x08, //   Usage Page (LED)
    0x95, 0x02, //   Report Count............ (2)
    0x75, 0x01, //   Report Size............. (1)
    0x19, 0x01, //   Usage Minimum........... (1)
    0x29, 0x02, //   Usage Maximum........... (2)
    0x91, 0x02, //   Output..................(Data, Variable, Absolute)
    0x95, 0x01, //   Report Count............ (1)
    0x75, 0x06, //   Report Size............. (6)
    0x91, 0x01, //   Output..................(Constant)
    0xc0,       // End Collection

    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xa1, 0x01, // Collection (Application)
    0x85, 0x06, //   Report Id (6)
    0x05, 0x08, //   Usage Page (LED)
    0x95, 0x02, //   Report Count............ (2)
    0x75, 0x01, //   Report Size............. (1)
    0x19, 0x01, //   Usage Minimum........... (1)
    0x29, 0x02, //   Usage Maximum........... (2)
    0x81, 0x02, //   Input...................(Data, Variable, Absolute)
    0x95, 0x01, //   Report Count............ (1)
    0x75, 0x06, //   Report Size............. (6)
    0x81, 0x01, //   Input...................(Constant)
    0xc0,       // End Collection
};
}

struct KarabinerDriverKitVirtualHIDKeyboard_IVars {
};

bool KarabinerDriverKitVirtualHIDKeyboard::init() {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard init");

  auto result = super::init();
  if (!result) {
    return false;
  }

  ivars = IONewZero(KarabinerDriverKitVirtualHIDKeyboard_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void KarabinerDriverKitVirtualHIDKeyboard::free() {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard free");

  IOSafeDeleteNULL(ivars, KarabinerDriverKitVirtualHIDKeyboard_IVars, 1);
}

bool KarabinerDriverKitVirtualHIDKeyboard::handleStart(IOService* provider) {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard handleStart");

  if (!super::handleStart(provider)) {
    os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard super::handleStart failed");
    return false;
  }

  return true;
}

kern_return_t IMPL(KarabinerDriverKitVirtualHIDKeyboard, Stop) {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard Stop");

  return Stop(provider, SUPERDISPATCH);
}

OSDictionary* KarabinerDriverKitVirtualHIDKeyboard::newDeviceDescription(void) {
  auto dictionary = OSDictionary::withCapacity(10);
  if (!dictionary) {
    os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard OSDictionary::withCapacity failed");
    return nullptr;
  }

  if (auto manufacturer = OSString::withCString("pqrs.org")) {
    OSDictionarySetValue(dictionary, kIOHIDManufacturerKey, manufacturer);
    manufacturer->release();
  }

  return dictionary;
}

OSData* KarabinerDriverKitVirtualHIDKeyboard::newReportDescriptor(void) {
  return OSData::withBytes(reportDescriptor_, sizeof(reportDescriptor_));
}
