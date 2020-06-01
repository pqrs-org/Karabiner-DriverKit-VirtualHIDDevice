#include "org_pqrs_KarabinerDriverKitVirtualHIDKeyboard.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "version.hpp"
#include <HIDDriverKit/IOHIDDeviceKeys.h>
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define LOG_PREFIX "KarabinerDriverKitVirtualHIDKeyboard " KARABINER_DRIVERKIT_VERSION

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

struct org_pqrs_KarabinerDriverKitVirtualHIDKeyboard_IVars {
  IOService* provider;
  OSDictionaryPtr properties;
};

bool org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  OSSafeReleaseNULL(ivars->properties);

  IOSafeDeleteNULL(ivars, org_pqrs_KarabinerDriverKitVirtualHIDKeyboard_IVars, 1);

  super::free();
}

bool org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::handleStart(IOService* provider) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " handleStart");

  ivars->provider = provider;

  if (!super::handleStart(provider)) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " super::handleStart failed");
    return false;
  }

  if (auto key = OSString::withCString("HIDDefaultBehavior")) {
    setProperty(key, kOSBooleanTrue);
    key->release();
  }

  if (auto key = OSString::withCString("AppleVendorSupported")) {
    setProperty(key, kOSBooleanTrue);
    key->release();
  }

  // Debug output

  {
    auto kr = CopyProperties(&(ivars->properties));
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " CopyProperties failed:0x%x", kr);
      return false;
    }

    if (ivars->properties) {
      if (auto userClientProperties = OSDynamicCast(OSDictionary, OSDictionaryGetValue(ivars->properties, "UserClientProperties"))) {
        if (auto s = OSDynamicCast(OSString, OSDictionaryGetValue(userClientProperties, "IOClass"))) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " UserClientProperties::IOClass %{public}s", s->getCStringNoCopy());
        }
        if (auto s = OSDynamicCast(OSString, OSDictionaryGetValue(userClientProperties, "IOUserClass"))) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " UserClientProperties::IOUserClass %{public}s", s->getCStringNoCopy());
        }
      }
    }
  }

  RegisterService();

  return true;
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  ivars->provider = nullptr;

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, NewUserClient) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " NewUserClient type:%d", type);

  IOService* client;

  auto kr = Create(this, "UserClientProperties", &client);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
    return kr;
  }

  os_log(OS_LOG_DEFAULT, LOG_PREFIX " UserClient is created");

  *userClient = OSDynamicCast(IOUserClient, client);
  if (!*userClient) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
    client->release();
    return kIOReturnError;
  }

  return kIOReturnSuccess;
}

OSDictionary* org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::newDeviceDescription(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newDeviceDescription");

  auto dictionary = OSDictionary::withCapacity(10);
  if (!dictionary) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDictionary::withCapacity failed");
    return nullptr;
  }

  if (auto manufacturer = OSString::withCString("pqrs.org")) {
    OSDictionarySetValue(dictionary, kIOHIDManufacturerKey, manufacturer);
    manufacturer->release();
  }

  if (auto product = OSString::withCString("Karabiner DriverKit VirtualHIDKeyboard " KARABINER_DRIVERKIT_VERSION)) {
    OSDictionarySetValue(dictionary, kIOHIDProductKey, product);
    product->release();
  }

  if (auto serialNumber = OSString::withCString("pqrs.org:KarabinerDriverKitVirtualHIDKeyboard")) {
    OSDictionarySetValue(dictionary, kIOHIDSerialNumberKey, serialNumber);
    serialNumber->release();
  }

  if (auto vendorId = OSNumber::withNumber(static_cast<uint32_t>(0x16c0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDVendorIDKey, vendorId);
    vendorId->release();
  }

  if (auto productId = OSNumber::withNumber(static_cast<uint32_t>(0x27db), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDProductIDKey, productId);
    productId->release();
  }

  if (auto locationId = OSNumber::withNumber(static_cast<uint32_t>(0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDLocationIDKey, locationId);
    locationId->release();
  }

  if (auto countryCode = OSNumber::withNumber(static_cast<uint32_t>(0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDCountryCodeKey, countryCode);
    countryCode->release();
  }

  if (auto usagePage = OSNumber::withNumber(static_cast<uint32_t>(kHIDPage_GenericDesktop), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDPrimaryUsagePageKey, usagePage);
    usagePage->release();
  }

  if (auto usage = OSNumber::withNumber(static_cast<uint32_t>(kHIDUsage_GD_Keyboard), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDPrimaryUsageKey, usage);
    usage->release();
  }

  return dictionary;
}

OSData* org_pqrs_KarabinerDriverKitVirtualHIDKeyboard::newReportDescriptor(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newReportDescriptor");

  return OSData::withBytes(reportDescriptor_, sizeof(reportDescriptor_));
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, postKeyboardInputReport) {
  if (!report) {
    return kIOReturnBadArgument;
  }

  uint64_t reportLength;
  auto kr = report->GetLength(&reportLength);
  if (kr != kIOReturnSuccess) {
    return kr;
  }

  return handleReport(mach_absolute_time(),
                      report,
                      static_cast<uint32_t>(reportLength),
                      kIOHIDReportTypeInput,
                      0);
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, reset) {
  //kern_return_t KarabinerDriverKitVirtualHIDKeyboard::reset(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " reset");

  return kIOReturnSuccess;
}
