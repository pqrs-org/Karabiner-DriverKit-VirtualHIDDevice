#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=2772A6CA4AFC07DF97C2BB1BFD92EA182BF4B2B5

set -e # forbid command failure

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements DriverKit/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/KarabinerDriverKitVirtualHIDDevice.app/Contents/Library/SystemExtensions/org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard.dext

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements ExtensionManager/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/KarabinerDriverKitVirtualHIDDevice.app
