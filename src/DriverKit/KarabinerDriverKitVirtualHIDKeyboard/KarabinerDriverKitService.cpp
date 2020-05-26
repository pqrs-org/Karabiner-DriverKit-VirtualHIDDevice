#include <DriverKit/IOLib.h>
#include <DriverKit/IOUserClient.h>
#include <DriverKit/IOUserServer.h>
#include <DriverKit/OSCollections.h>
#include <os/log.h>

#include "KarabinerDriverKitService.h"
#include "version.hpp"

#define LOG_PREFIX "KarabinerDriverKitService " KARABINER_DRIVERKIT_VERSION

struct KarabinerDriverKitService_IVars {
  OSDictionaryPtr properties;
};

bool KarabinerDriverKitService::init() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " init");

  auto result = super::init();
  if (!result) {
    return false;
  }

  ivars = IONewZero(KarabinerDriverKitService_IVars, 1);
  if (ivars == nullptr) {
    return false;
  }

  return true;
}

void KarabinerDriverKitService::free() {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " free");

  OSSafeReleaseNULL(ivars->properties);

  IOSafeDeleteNULL(ivars, KarabinerDriverKitService_IVars, 1);

  super::free();
}

kern_return_t IMPL(KarabinerDriverKitService, Start) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start");

  {
    auto kr = Start(provider, SUPERDISPATCH);
    if (kr != kIOReturnSuccess) {
      os_log(OS_LOG_DEFAULT, LOG_PREFIX " Start failed");
      return false;
    }
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

kern_return_t IMPL(KarabinerDriverKitService, Stop) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " Stop");

  return Stop(provider, SUPERDISPATCH);
}

kern_return_t IMPL(KarabinerDriverKitService, NewUserClient) {
  os_log(OS_LOG_DEFAULT, LOG_PREFIX " NewUserClient type:%d", type);

  IOService* client;

  auto kr = Create(this, "UserClientProperties", &client);
  if (kr != kIOReturnSuccess) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOService::Create failed: 0x%x", kr);
    return kr;
  }

  os_log(OS_LOG_DEFAULT, LOG_PREFIX " IOUserUserClient is created");

  *userClient = OSDynamicCast(IOUserClient, client);
  if (!*userClient) {
    os_log(OS_LOG_DEFAULT, LOG_PREFIX " OSDynamicCast failed");
    client->release();
    return kIOReturnError;
  }

  return kIOReturnSuccess;
}
