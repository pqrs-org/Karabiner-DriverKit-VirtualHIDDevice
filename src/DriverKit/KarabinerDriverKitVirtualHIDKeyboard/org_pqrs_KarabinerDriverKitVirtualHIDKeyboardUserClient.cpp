#include <DriverKit/IOLib.h>
#include <DriverKit/IOUserClient.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/OSCollections.h>
#include <os/log.h>

#include "KarabinerDriverKitVirtualHIDKeyboard.h"
#include "org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient.h"
#include "pqrs/karabiner/driverkit/virtual_hid_device.hpp"
#include "version.hpp"

#define LOG_PREFIX "KarabinerDriverKitVirtualHIDKeyboardUserClient " KARABINER_DRIVERKIT_VERSION

struct org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient_IVars {
  KarabinerDriverKitVirtualHIDKeyboard* keyboard;
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

  ivars->keyboard = OSDynamicCast(KarabinerDriverKitVirtualHIDKeyboard, provider);
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
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " ExternalMethod %llu", selector);

  switch (pqrs::karabiner::driverkit::virtual_hid_device::user_client_method(selector)) {
    case pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::post_keyboard_input_report:
      return ivars->keyboard->postKeyboardInputReport(selector);
    case pqrs::karabiner::driverkit::virtual_hid_device::user_client_method::reset_virtual_hid_keyboard:
      return ivars->keyboard->reset();
    default:
      break;
  }

  return kIOReturnSuccess;
}
