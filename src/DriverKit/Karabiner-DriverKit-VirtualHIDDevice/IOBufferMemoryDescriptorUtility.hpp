#pragma once

#include <DriverKit/DriverKit.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <DriverKit/OSCollections.h>

namespace IOBufferMemoryDescriptorUtility {

inline kern_return_t createWithBytes(const void* bytes, size_t length, IOMemoryDescriptor** memory) {
  if (!bytes || !memory) {
    return kIOReturnBadArgument;
  }

  IOBufferMemoryDescriptor* m = nullptr;
  auto kr = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, length, 0, &m);
  if (kr != kIOReturnSuccess) {
    goto error;
  }

  uint64_t address;
  uint64_t len;
  kr = m->Map(0, 0, 0, 0, &address, &len);
  if (kr != kIOReturnSuccess) {
    goto error;
  }

  if (length != len) {
    kr = kIOReturnNoMemory;
    goto error;
  }

  memcpy(reinterpret_cast<void*>(address), bytes, length);

  *memory = m;

  return kr;

error:
  if (m) {
    OSSafeReleaseNULL(m);
  }

  return kr;
}

} // namespace IOBufferMemoryDescriptorUtility
