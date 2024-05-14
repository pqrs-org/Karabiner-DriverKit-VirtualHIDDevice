#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=BD3B995B69EBA8FC153B167F063079D19CCC2834

set -e # forbid command failure

#
# Sign Karabiner-DriverKit-VirtualHIDDevice.dext
#

# Embed provisioning profile
cp \
    DriverKit/Developer_ID_KarabinerDriverKitVirtualHIDDevice.provisionprofile \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext/embedded.provisionprofile

# Sign
codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements DriverKit/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext

#
# Sign Karabiner-VirtualHIDDevice-Manager.app
#

# Embed provisioning profile
cp \
    Manager/Developer_ID_Karabiner_VirtualHIDDevice_Manager.provisionprofile \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/embedded.provisionprofile

# Sign
codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements Manager/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app

#
# Sign Karabiner-DriverKit-Daemon
#

# Embed provisioning profile
cp \
    Daemon/Developer_ID_KarabinerDriverKitDaemon.provisionprofile \
    Daemon/build/Release/Karabiner-DriverKit-Daemon.app/Contents/embedded.provisionprofile

# Sign
codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements Daemon/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    Daemon/build/Release/Karabiner-DriverKit-Daemon.app
