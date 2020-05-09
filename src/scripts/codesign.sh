#!/bin/bash

# Replace with your identity
code_sign_identity=2772A6CA4AFC07DF97C2BB1BFD92EA182BF4B2B5

codesign \
    --sign $code_sign_identity \
    --entitlements DriverKit/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/KarabinerDriverKitVirtualHIDDevice.app/Contents/Library/SystemExtensions/org.pqrs.driverkit.KarabinerDriverKitVirtualHIDKeyboard.dext

codesign \
    --sign $code_sign_identity \
    --entitlements ExtensionManager/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/KarabinerDriverKitVirtualHIDDevice.app
