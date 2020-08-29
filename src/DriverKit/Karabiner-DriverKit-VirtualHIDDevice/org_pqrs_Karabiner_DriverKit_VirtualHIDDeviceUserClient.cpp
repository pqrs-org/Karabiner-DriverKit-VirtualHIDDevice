#include "org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard.h"
#include "org_pqrs_Karabiner_DriverKit_VirtualHIDPointing.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device_driver.hpp"
#include "version.hpp"
#include <os/log.h>

#define LOG_PREFIX "Karabiner-DriverKit-VirtualHIDDeviceUserClient " KARABINER_DRIVERKIT_VERSION

namespace {
kern_return_t createIOMemoryDescriptor(IOUserClientMethodArguments* arguments, IOMemoryDescriptor** memory) {
  if (!memory) {
    return kIOReturnBadArgument;
  }

  *memory = nullptr;

  if (arguments->structureInput) {
    auto kr = IOBufferMemoryDescriptorUtility::createWithBytes(arguments->structureInput->getBytesNoCopy(),
                                                               arguments->structureInput->getLength(),
                                                               memory);
    if (kr != kIOReturnSuccess) {
      return kr;
    }
  } else if (arguments->structureInputDescriptor) {
    *memory = arguments->structureInputDescriptor;
    (*memory)->retain();
  }

  return kIOReturnSuccess;
}
} // namespace

struct org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars {
  uint32_t keyboardCountryCode;
  org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard* keyboard;
  bool keyboardReady;
  org_pqrs_Karabiner_DriverKit_VirtualHIDPointing* pointing;
  bool pointingReady;
};

bool org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  OSSafeReleaseNULL(ivars->keyboard);
  OSSafeReleaseNULL(ivars->pointing);

  IOSafeDeleteNULL(ivars, org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient_IVars, 1);

  super::free();
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, Start) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start");

  {
    auto kr = Start(provider, SUPERDISPATCH);
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start failed");
      return kr;
    }
  }

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient::ExternalMethod(uint64_t selector,
                                                                                      IOUserClientMethodArguments* arguments,
                                                                                      const IOUserClientMethodDispatch* dispatch,
                                                                                      OSObject* target,
                                                                                      void* reference) {
  switch (pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method(selector)) {
    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_initialize:
      if (!ivars->keyboard) {
        ivars->keyboardReady = false;

        if (arguments->scalarInputCount > 0) {
          ivars->keyboardCountryCode = static_cast<uint32_t>(arguments->scalarInput[0]);
        }

        IOService* client;

        auto kr = Create(this, "VirtualHIDKeyboardProperties", &client);
        if (kr != kIOReturnSuccess) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
          return kr;
        }

        ivars->keyboard = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDKeyboard, client);
        if (!ivars->keyboard) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
          client->release();
          return kIOReturnError;
        }
      }
      return kIOReturnSuccess;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_terminate:
      ivars->keyboardReady = false;
      OSSafeReleaseNULL(ivars->keyboard);
      return kIOReturnSuccess;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_ready:
      if (arguments->scalarOutput && arguments->scalarOutputCount > 0) {
        arguments->scalarOutput[0] = ivars->keyboardReady;
        return kIOReturnSuccess;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_post_report:
      if (ivars->keyboard) {
        IOMemoryDescriptor* memory = nullptr;

        auto kr = createIOMemoryDescriptor(arguments, &memory);
        if (kr == kIOReturnSuccess) {
          kr = ivars->keyboard->postReport(memory);
          OSSafeReleaseNULL(memory);
        }

        return kr;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_keyboard_reset:
      if (ivars->keyboard) {
        return ivars->keyboard->reset();
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_initialize:
      if (!ivars->pointing) {
        ivars->pointingReady = false;

        IOService* client;

        auto kr = Create(this, "VirtualHIDPointingProperties", &client);
        if (kr != kIOReturnSuccess) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
          return kr;
        }

        ivars->pointing = OSDynamicCast(org_pqrs_Karabiner_DriverKit_VirtualHIDPointing, client);
        if (!ivars->pointing) {
          os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
          client->release();
          return kIOReturnError;
        }
      }
      return kIOReturnSuccess;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_terminate:
      ivars->pointingReady = false;
      OSSafeReleaseNULL(ivars->pointing);
      return kIOReturnSuccess;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_ready:
      if (arguments->scalarOutput && arguments->scalarOutputCount > 0) {
        arguments->scalarOutput[0] = ivars->pointingReady;
        return kIOReturnSuccess;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_post_report:
      if (ivars->pointing) {
        IOMemoryDescriptor* memory = nullptr;

        auto kr = createIOMemoryDescriptor(arguments, &memory);
        if (kr == kIOReturnSuccess) {
          kr = ivars->pointing->postReport(memory);
          OSSafeReleaseNULL(memory);
        }

        return kr;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device_driver::user_client_method::virtual_hid_pointing_reset:
      if (ivars->pointing) {
        return ivars->pointing->reset();
      }
      return kIOReturnError;

    default:
      break;
  }

  return kIOReturnBadArgument;
}

void IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, setKeyboardReady) {
  ivars->keyboardReady = value;
}

void IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, setPointingReady) {
  ivars->pointingReady = value;
}

uint32_t IMPL(org_pqrs_Karabiner_DriverKit_VirtualHIDDeviceUserClient, getKeyboardCountryCode) {
  return ivars->keyboardCountryCode;
}
