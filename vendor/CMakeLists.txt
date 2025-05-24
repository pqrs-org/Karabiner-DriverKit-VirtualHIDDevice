cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(vendor NONE)

include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/vendor/CPM.cmake")

CPMUsePackageLock("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/package-lock.cmake")

set(VENDOR_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor")
file(MAKE_DIRECTORY "${VENDOR_INCLUDE_DIR}")

include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/dependency.cmake")

copy_vendor_package(glob)
copy_vendor_package(gsl)
copy_vendor_package(pqrs_hid)
copy_vendor_package(pqrs_local_datagram)
copy_vendor_package(pqrs_osx_iokit_types)
copy_vendor_package(pqrs_osx_iokit_return)
copy_vendor_package(pqrs_osx_iokit_service_monitor)
copy_vendor_package(pqrs_osx_process_info)
copy_vendor_package(pqrs_spdlog)
copy_vendor_package(ut)
