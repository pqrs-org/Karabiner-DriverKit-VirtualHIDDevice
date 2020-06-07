#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=2772A6CA4AFC07DF97C2BB1BFD92EA182BF4B2B5

set -e # forbid command failure

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --entitlements Karabiner-DriverKit-VirtualHIDKeyboard-Client/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-DriverKit-VirtualHIDKeyboard-Client.app
