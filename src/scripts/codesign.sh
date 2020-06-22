#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=C6DD0BCD24C737EA0505F1EB26B8BBEEDEC12F1B

set -e # forbid command failure

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements DriverKit/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions/org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard.dext

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements ExtensionManager/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-DriverKit-ExtensionManager.app
