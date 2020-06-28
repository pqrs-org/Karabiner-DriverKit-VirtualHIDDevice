#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=C6DD0BCD24C737EA0505F1EB26B8BBEEDEC12F1B

set -e # forbid command failure

# Embed provisioning profile
cp \
    Karabiner-DriverKit-VirtualHIDDeviceClient/embedded.provisionprofile \
    build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app/Contents/embedded.provisionprofile

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements Karabiner-DriverKit-VirtualHIDDeviceClient/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app
