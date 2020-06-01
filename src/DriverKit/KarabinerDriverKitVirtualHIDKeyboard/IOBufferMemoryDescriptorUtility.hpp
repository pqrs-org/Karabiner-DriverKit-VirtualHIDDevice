#pragma once

#include <DriverKit/DriverKit.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <DriverKit/OSCollections.h>

namespace IOBufferMemoryDescriptorUtility {

inline kern_return_t createWithData(OSData* data, IOMemoryDescriptor** memory) {
  if (!data || !memory) {
    return kIOReturnBadArgument;
  }

  IOBufferMemoryDescriptor* m = nullptr;
  auto kr = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, data->getLength(), 0, &m);
  if (kr == kIOReturnSuccess) {
    uint64_t address;
    uint64_t length;
    m->Map(0, 0, 0, 0, &address, &length);

    if (data->getLength() != length) {
      kr = kIOReturnNoMemory;
      goto error;
    }

    memcpy(reinterpret_cast<void*>(address), data->getBytesNoCopy(), length);
  }

  *memory = m;

  return kr;

error:
  if (m) {
    OSSafeReleaseNULL(m);
  }

  return kr;
}

} // namespace IOBufferMemoryDescriptorUtility
