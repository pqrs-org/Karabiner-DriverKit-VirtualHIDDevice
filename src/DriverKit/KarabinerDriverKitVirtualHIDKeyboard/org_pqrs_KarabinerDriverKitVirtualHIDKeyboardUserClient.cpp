#include "org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient.h"
#include "IOBufferMemoryDescriptorUtility.hpp"
#include "org_pqrs_KarabinerDriverKitVirtualHIDKeyboard.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device.hpp"
#include "version.hpp"
#include <os/log.h>

#define LOG_PREFIX "KarabinerDriverKitVirtualHIDKeyboardUserClient " KARABINER_DRIVERKIT_VERSION

struct org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient_IVars {
  org_pqrs_KarabinerDriverKitVirtualHIDKeyboard* keyboard;
};

bool org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  if (!super::init()) {
    return false;
  }

  ivars = IONewZero(org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  IOSafeDeleteNULL(ivars, org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient_IVars, 1);

  super::free();
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient, Start) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start");

  {
    auto kr = Start(provider, SUPERDISPATCH);
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start failed");
      return kr;
    }
  }

  ivars->keyboard = OSDynamicCast(org_pqrs_KarabinerDriverKitVirtualHIDKeyboard, provider);
  if (ivars->keyboard) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " provider == KarabinerDriverKitVirtualHIDKeyboard");
  } else {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " provider != KarabinerDriverKitVirtualHIDKeyboard");
    return kIOReturnBadArgument;
  }

  return kIOReturnSuccess;
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  ivars->keyboard = nullptr;

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::ExternalMethod(uint64_t selector,
                                                                                      IOUserClientMethodArguments* arguments,
                                                                                      const IOUserClientMethodDispatch* dispatch,
                                                                                      OSObject* target,
                                                                                      void* reference) {
  switch (pqrs::karabiner::driverkit::virtual_hid_device::user_client_method(selector)) {
    case pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_post_report:
      if (ivars->keyboard) {
        IOMemoryDescriptor* memory = nullptr;

        if (arguments->structureInput) {
          auto kr = IOBufferMemoryDescriptorUtility::createWithBytes(arguments->structureInput->getBytesNoCopy(),
                                                                     arguments->structureInput->getLength(),
                                                                     &memory);
          if (kr != kIOReturnSuccess) {
            return kr;
          }
        } else if (arguments->structureInputDescriptor) {
          memory = arguments->structureInputDescriptor;
          memory->retain();
        }

        auto kr = ivars->keyboard->postReport(memory);

        OSSafeReleaseNULL(memory);

        return kr;
      }
      return kIOReturnError;

    case pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::virtual_hid_keyboard_reset:
      if (ivars->keyboard) {
        return ivars->keyboard->reset();
      }
      return kIOReturnError;

    default:
      break;
  }

  return kIOReturnBadArgument;
}
