#include <DriverKit/IOLib.h>
#include <DriverKit/IOUserClient.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/OSCollections.h>
#include <os/log.h>

#include "KarabinerDriverKitVirtualHIDKeyboard.h"
#include "org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient.h"
#include "version.hpp"

#define LOG_PREFIX "KarabinerDriverKitVirtualHIDKeyboardUserClient " KARABINER_DRIVERKIT_VERSION

struct org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient_IVars {
  KarabinerDriverKitVirtualHIDKeyboard* keyboard;
};

bool org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  auto result = super::init();
  if (!result) {
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
      return false;
    }
  }

  ivars->keyboard = OSDynamicCast(KarabinerDriverKitVirtualHIDKeyboard, provider);

  return true;
}

kern_return_t IMPL(org_pqrs_KarabinerDriverKitVirtualHIDKeyboardUserClient, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  ivars->keyboard = nullptr;

  return Stop(provider, SUPERDISPATCH);
}
