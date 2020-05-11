#include "KarabinerDriverKitVirtualHIDKeyboard.h"
#include <DriverKit/IOLib.h>
#include <DriverKit/IOUserServer.h>
#include <os/log.h>

struct KarabinerDriverKitVirtualHIDKeyboard_IVars {
};

bool KarabinerDriverKitVirtualHIDKeyboard::init() {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard %s", __FUNCTION__);

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
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard %s", __FUNCTION__);

  IOSafeDeleteNULL(ivars, KarabinerDriverKitVirtualHIDKeyboard_IVars, 1);
}

bool KarabinerDriverKitVirtualHIDKeyboard::handleStart(IOService* provider) {
  if (!super::handleStart(provider)) {
    os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard super::handleStart failed");
    return false;
  }

  return true;
}

kern_return_t IMPL(KarabinerDriverKitVirtualHIDKeyboard, Stop) {
  os_log(OS_LOG_DEFAULT, "org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard %s", __FUNCTION__);

  return Stop(provider, SUPERDISPATCH);
}
