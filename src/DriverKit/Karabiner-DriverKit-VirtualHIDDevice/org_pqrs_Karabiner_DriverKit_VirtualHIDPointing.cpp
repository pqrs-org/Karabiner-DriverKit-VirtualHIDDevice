#include "org_pqrs_Karabiner_DriverKit_VirtualHIDPointing.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp"
#include "version.hpp"
#include <HIDDriverKit/IOHIDDeviceKeys.h>
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define LOG_PREFIX "Karabiner-DriverKit-VirtualHIDPointing " KARABINER_DRIVERKIT_VERSION

namespace {
const uint8_t reportDescriptor[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,        // USAGE (Mouse)
    0xa1, 0x01,        // COLLECTION (Application)
    0x09, 0x02,        //   USAGE (Mouse)
    0xa1, 0x02,        //   COLLECTION (Logical)
    0x09, 0x01,        //     USAGE (Pointer)
    0xa1, 0x00,        //     COLLECTION (Physical)
    /*              */ // ------------------------------ Buttons
    0x05, 0x09,        //       USAGE_PAGE (Button)
    0x19, 0x01,        //       USAGE_MINIMUM (Button 1)
    0x29, 0x20,        //       USAGE_MAXIMUM (Button 32)
    0x15, 0x00,        //       LOGICAL_MINIMUM (0)
    0x25, 0x01,        //       LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //       REPORT_SIZE (1)
    0x95, 0x20,        //       REPORT_COUNT (32 Buttons)
    0x81, 0x02,        //       INPUT (Data,Var,Abs)
    /*              */ // ------------------------------ X,Y position
    0x05, 0x01,        //       USAGE_PAGE (Generic Desktop)
    0x09, 0x30,        //       USAGE (X)
    0x09, 0x31,        //       USAGE (Y)
    0x15, 0x81,        //       LOGICAL_MINIMUM (-127)
    0x25, 0x7f,        //       LOGICAL_MAXIMUM (127)
    0x75, 0x08,        //       REPORT_SIZE (8)
    0x95, 0x02,        //       REPORT_COUNT (2)
    0x81, 0x06,        //       INPUT (Data,Var,Rel)
    0xa1, 0x02,        //       COLLECTION (Logical)
    /*              */ // ------------------------------ Vertical wheel res multiplier
    0x09, 0x48,        //         USAGE (Resolution Multiplier)
    0x15, 0x00,        //         LOGICAL_MINIMUM (0)
    0x25, 0x01,        //         LOGICAL_MAXIMUM (1)
    0x35, 0x01,        //         PHYSICAL_MINIMUM (1)
    0x45, 0x04,        //         PHYSICAL_MAXIMUM (4)
    0x75, 0x02,        //         REPORT_SIZE (2)
    0x95, 0x01,        //         REPORT_COUNT (1)
    0xa4,              //         PUSH
    0xb1, 0x02,        //         FEATURE (Data,Var,Abs)
    /*              */ // ------------------------------ Vertical wheel
    0x09, 0x38,        //         USAGE (Wheel)
    0x15, 0x81,        //         LOGICAL_MINIMUM (-127)
    0x25, 0x7f,        //         LOGICAL_MAXIMUM (127)
    0x35, 0x00,        //         PHYSICAL_MINIMUM (0)        - reset physical
    0x45, 0x00,        //         PHYSICAL_MAXIMUM (0)
    0x75, 0x08,        //         REPORT_SIZE (8)
    0x81, 0x06,        //         INPUT (Data,Var,Rel)
    0xc0,              //       END_COLLECTION
    0xa1, 0x02,        //       COLLECTION (Logical)
    /*              */ // ------------------------------ Horizontal wheel res multiplier
    0x09, 0x48,        //         USAGE (Resolution Multiplier)
    0xb4,              //         POP
    0xb1, 0x02,        //         FEATURE (Data,Var,Abs)
    /*              */ // ------------------------------ Padding for Feature report
    0x35, 0x00,        //         PHYSICAL_MINIMUM (0)        - reset physical
    0x45, 0x00,        //         PHYSICAL_MAXIMUM (0)
    0x75, 0x04,        //         REPORT_SIZE (4)
    0xb1, 0x03,        //         FEATURE (Cnst,Var,Abs)
    /*              */ // ------------------------------ Horizontal wheel
    0x05, 0x0c,        //         USAGE_PAGE (Consumer Devices)
    0x0a, 0x38, 0x02,  //         USAGE (AC Pan)
    0x15, 0x81,        //         LOGICAL_MINIMUM (-127)
    0x25, 0x7f,        //         LOGICAL_MAXIMUM (127)
    0x75, 0x08,        //         REPORT_SIZE (8)
    0x81, 0x06,        //         INPUT (Data,Var,Rel)
    0xc0,              //       END_COLLECTION
    0xc0,              //     END_COLLECTION
    0xc0,              //   END_COLLECTION
    0xc0               // END_COLLECTION};
};
}

struct org_pqrs_Karabiner_DriverKit_VirtualHIDPointing_IVars {
  org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient* provider;
  bool ready;
};

bool org_pqrs_Karabiner_DriverKit_VirtualHIDPointing::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_Karabiner_DriverKit_VirtualHIDPointing::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  IOSafeDeleteNULL(ivars, org_pqrs_Karabiner_DriverKit_VirtualHIDPointing_IVars, 1);

  super::free();
}

bool org_pqrs_Karabiner_DriverKit_VirtualHIDPointing::handleStart(IOService* provider) {
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

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  ivars->provider = nullptr;

  return Stop(provider, SUPERDISPATCH);
}

OSDictionary* org_pqrs_Karabiner_DriverKit_VirtualHIDPointing::newDeviceDescription(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newDeviceDescription");

  auto dictionary = OSDictionary::withCapacity(10);
  if (!dictionary) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDictionary::withCapacity failed");
    return nullptr;
  }

  // Set kIOHIDRegisterServiceKey in order to call registerService in IOHIDDevice::start.
  OSDictionarySetValue(dictionary, "RegisterService", kOSBooleanTrue);
  OSDictionarySetValue(dictionary, "HIDDefaultBehavior", kOSBooleanTrue);

  if (auto manufacturer = OSString::withCString("pqrs.org")) {
    OSDictionarySetValue(dictionary, kIOHIDManufacturerKey, manufacturer);
    manufacturer->release();
  }

  if (auto product = OSString::withCString("Karabiner DriverKit VirtualHIDPointing " KARABINER_DRIVERKIT_VERSION)) {
    OSDictionarySetValue(dictionary, kIOHIDProductKey, product);
    product->release();
  }

  if (auto serialNumber = OSString::withCString("pqrs.org:Karabiner-DriverKit-VirtualHIDPointing")) {
    OSDictionarySetValue(dictionary, kIOHIDSerialNumberKey, serialNumber);
    serialNumber->release();
  }

  if (auto vendorId = OSNumber::withNumber(static_cast<uint32_t>(0x16c0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDVendorIDKey, vendorId);
    vendorId->release();
  }

  if (auto productId = OSNumber::withNumber(static_cast<uint32_t>(0x27da), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDProductIDKey, productId);
    productId->release();
  }

  if (auto locationId = OSNumber::withNumber(static_cast<uint32_t>(0), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDLocationIDKey, locationId);
    locationId->release();
  }

  if (auto usagePage = OSNumber::withNumber(static_cast<uint32_t>(kHIDPage_GenericDesktop), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDPrimaryUsagePageKey, usagePage);
    usagePage->release();
  }

  if (auto usage = OSNumber::withNumber(static_cast<uint32_t>(kHIDUsage_GD_Mouse), 32)) {
    OSDictionarySetValue(dictionary, kIOHIDPrimaryUsageKey, usage);
    usage->release();
  }

  return dictionary;
}

OSData* org_pqrs_Karabiner_DriverKit_VirtualHIDPointing::newReportDescriptor(void) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " newReportDescriptor");

  return OSData::withBytes(reportDescriptor, sizeof(reportDescriptor));
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing, postReport) {
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

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing, reset) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " reset");

  // Post empty reports

  pqrs::karabiner::driverkit::virtual_hid_device_driver::hid_report::pointing_input pointing_input;

  struct input {
    const void* address;
    size_t length;
  } inputs[] = {
      {&pointing_input, sizeof(pointing_input)},
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

bool IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing, getReady) {
  return ivars->ready;
}
