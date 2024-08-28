#include "org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp"
#include "version.hpp"
#include <HIDDriverKit/IOHIDDeviceKeys.h>
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define LOG_PREFIX "Karabiner-DriverKit-VirtualHIDKeyboard " KARABINER_DRIVERKIT_VERSION

namespace {

//
// Note:
// Too large usage maximum, e.g. 2048, causes high CPU usage with `ioreg -l ` on macOS Monterey 12.0.1.
// Thus, we have to set a smallest value to usage maximum.
//

const uint8_t reportDescriptor[] = {
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
    0x75, 0x10,       //   Report Size............. (16)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x2a, 0xff, 0x00, //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x05, 0x0c,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage 1 (kHIDUsage_Csmr_ConsumerControl)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x02,       //   Report Id (2)
    0x05, 0x0c,       //   Usage Page (Consumer)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x10,       //   Report Size............. (16)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0x00, 0x03, //   Logical Maximum......... (768) (Consumer usage 29d-ffff is reserved)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x2a, 0x00, 0x03, //   Usage Maximum........... (768)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x06, 0x00, 0xff, // Usage Page (kHIDPage_AppleVendor)
    0x09, 0x01,       // Usage 1 (kHIDUsage_AppleVendor_TopCase)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x03,       //   Report Id (3)
    0x05, 0xff,       //   Usage Page (kHIDPage_AppleVendorTopCase)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x10,       //   Report Size............. (16)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x2a, 0xff, 0x00, //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection

    0x06, 0x00, 0xff, // Usage Page (kHIDPage_AppleVendor)
    0x09, 0x06,       // Usage 6 (kHIDUsage_AppleVendor_Keyboard)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x04,       //   Report Id (4)
    0x06, 0x01, 0xff, //   Usage Page (kHIDPage_AppleVendorKeyboard)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x10,       //   Report Size............. (16)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x2a, 0xff, 0x00, //   Usage Maximum........... (255)
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

    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xa1, 0x01,       // Collection (Application)
    0x85, 0x07,       //   Report Id (7)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x95, 0x20,       //   Report Count............ (32)
    0x75, 0x10,       //   Report Size............. (16)
    0x15, 0x00,       //   Logical Minimum......... (0)
    0x26, 0xff, 0x00, //   Logical Maximum......... (255)
    0x19, 0x00,       //   Usage Minimum........... (0)
    0x2a, 0xff, 0x00, //   Usage Maximum........... (255)
    0x81, 0x00,       //   Input...................(Data, Array, Absolute)
    0xc0,             // End Collection
};
} // namespace

struct org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard_IVars {
  org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient* provider;
  bool ready;
  uint8_t lastLedState;
};

bool org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  IOSafeDeleteNULL(ivars, org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard_IVars, 1);

  super::free();
}

bool org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::handleStart(IOService* provider) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " handleStart");

  ivars->provider = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, provider);
  if (!ivars->provider) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " provider is not org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient");
    return false;
  }

  if (!super::handleStart(provider)) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " super::handleStart failed");
    return false;
  }

  ivars->ready = true;

  return true;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  ivars->provider = nullptr;

  return Stop(provider, SUPERDISPATCH);
}

OSDictionary* org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::newDeviceDescription(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newDeviceDescription");

  auto dictionary = OSDictionary::withCapacity(12);
  if (!dictionary) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDictionary::withCapacity failed");
    return nullptr;
  }

  // Set kIOHIDRegisterServiceKey in order to call registerService in IOHIDDevice::start.
  OSDictionarySetValue(dictionary, "RegisterService", kOSBooleanTrue);
  OSDictionarySetValue(dictionary, "HIDDefaultBehavior", kOSBooleanTrue);
  OSDictionarySetValue(dictionary, "AppleVendorSupported", kOSBooleanTrue);

  if (auto manufacturer = OSString::withCString("pqrs.org")) {
    OSDictionarySetValue(dictionary, kIOHIDManufacturerKey, manufacturer);
    manufacturer->release();
  }

  if (auto product = OSString::withCString("Karabiner DriverKit VirtualHIDKeyboard " KARABINER_DRIVERKIT_VERSION)) {
    OSDictionarySetValue(dictionary, kIOHIDProductKey, product);
    product->release();
  }

  if (auto serialNumber = OSString::withCString("pqrs.org:Karabiner-DriverKit-VirtualHIDKeyboard")) {
    OSDictionarySetValue(dictionary, kIOHIDSerialNumberKey, serialNumber);
    serialNumber->release();
  }

  uint32_t keyboardVendorId = 0x16c0;
  if (ivars->provider) {
    keyboardVendorId = ivars->provider->getKeyboardVendorId();
  }
  if (auto vendorId = OSNumber::withNumber(keyboardVendorId, 32)) {
    OSDictionarySetValue(dictionary, kIOHIDVendorIDKey, vendorId);
    vendorId->release();
  }

  uint32_t keyboardProductId = 0x27db;
  if (ivars->provider) {
    keyboardProductId = ivars->provider->getKeyboardProductId();
  }
  if (auto productId = OSNumber::withNumber(keyboardProductId, 32)) {
    OSDictionarySetValue(dictionary, kIOHIDProductIDKey, productId);
    productId->release();
  }

  if (auto locationId = OSNumber::withNumber(static_cast<uint32_t>(0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDLocationIDKey, locationId);
    locationId->release();
  }

  uint32_t keyboardCountryCode = 0;
  if (ivars->provider) {
    keyboardCountryCode = ivars->provider->getKeyboardCountryCode();
  }
  if (auto countryCode = OSNumber::withNumber(keyboardCountryCode, 32)) {
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

OSData* org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::newReportDescriptor(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newReportDescriptor");

  return OSData::withBytes(reportDescriptor, sizeof(reportDescriptor));
}

kern_return_t org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard::setReport(IOMemoryDescriptor* report,
                                                                         IOHIDReportType reportType,
                                                                         IOOptionBits options,
                                                                         uint32_t completionTimeout,
                                                                         OSAction* action) {
  if (!report) {
    return kIOReturnBadArgument;
  }

  uint64_t address = 0;
  uint64_t len = 0;
  auto kr = report->Map(0, 0, 0, 0, &address, &len);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " setReport Map error: 0x%x", kr);
    return kr;
  }

  if (len < 2) {
    return kIOReturnBadArgument;
  }

  // The reportId is described at `reportDescriptor`.
  auto reportId = reinterpret_cast<uint8_t*>(address)[0];
  // state bits: 0b000000(caps lock)(num lock)
  auto state = reinterpret_cast<uint8_t*>(address)[1];

  if (reportId != 5) {
    // Error unless LED report.
    return kIOReturnUnsupported;
  }

  if (ivars->lastLedState == state) {
    return kIOReturnSuccess;
  }

  ivars->lastLedState = state;

  struct __attribute__((packed)) ledReport {
    uint8_t reportId;
    uint8_t state;
  } ledReport;

  ledReport.reportId = 6;
  ledReport.state = state;

  // Post LED report.

  IOMemoryDescriptor* memory = nullptr;

  kr = IOBufferMemoryDescriptorUtility::createWithBytes(&ledReport,
                                                        sizeof(ledReport),
                                                        &memory);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " setReport createWithBytes error: 0x%x", kr);
    return kr;
  }

  postReport(memory);

  OSSafeReleaseNULL(memory);

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, postReport) {
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

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, reset) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " reset");

  // Post empty reports

  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::keyboard_input keyboard_input;
  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::consumer_input consumer_input;
  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_keyboard_input apple_vendor_keyboard_input;
  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::apple_vendor_top_case_input apple_vendor_top_case_input;
  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::generic_desktop_input generic_desktop_input;

  struct input {
    const void* address;
    size_t length;
  } inputs[] = {
      {&keyboard_input, sizeof(keyboard_input)},
      {&consumer_input, sizeof(consumer_input)},
      {&apple_vendor_keyboard_input, sizeof(apple_vendor_keyboard_input)},
      {&apple_vendor_top_case_input, sizeof(apple_vendor_top_case_input)},
      {&generic_desktop_input, sizeof(generic_desktop_input)},
  };

  for (const auto& input : inputs) {
    IOMemoryDescriptor* memory = nullptr;
    auto kr = IOBufferMemoryDescriptorUtility::createWithBytes(input.address,
                                                               input.length,
                                                               &memory);
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " reset createWithBytes error: 0x%x", kr);
      return kr;
    }

    postReport(memory);

    OSSafeReleaseNULL(memory);
  }

  return kIOReturnSuccess;
}

bool IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, getReady) {
  return ivars->ready;
}
