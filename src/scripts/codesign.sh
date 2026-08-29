#!/bin/bash

set -u # forbid undefined variables
set -e # forbid command failure

readonly PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

trap "printf '\033[0m'" EXIT

set_normal_color() {
    printf '\033[32;49m'
}

set_error_color() {
    printf '\033[31;49m'
}

run_with_result_color() {
    local output
    local status

    if output=$("$@" 2>&1); then
        set_normal_color
        if [[ -n $output ]]; then
            printf '%s\n' "$output"
        fi
    else
        status=$?
        set_error_color
        if [[ -n $output ]]; then
            printf '%s\n' "$output" >&2
        fi
        return "$status"
    fi
}

set_normal_color

readonly CODE_SIGN_IDENTITY=$(bash $(dirname $0)/../../scripts/get-codesign-identity.sh)

if [[ -z $CODE_SIGN_IDENTITY ]]; then
    echo "Skip codesign"
    exit 0
fi

#
# Sign Karabiner-DriverKit-VirtualHIDDevice.dext
#

# Embed provisioning profile
cp \
    DriverKit/Developer_ID_KarabinerDriverKitVirtualHIDDevice.provisionprofile \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext/embedded.provisionprofile

# Sign
run_with_result_color codesign \
    --sign "$CODE_SIGN_IDENTITY" \
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
    Manager/Developer_ID_KarabinerVirtualHIDDeviceManager.provisionprofile \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app/Contents/embedded.provisionprofile

# Sign
run_with_result_color codesign \
    --sign "$CODE_SIGN_IDENTITY" \
    --entitlements Manager/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app

#
# Sign Karabiner-VirtualHIDDevice-Daemon
#

# Embed provisioning profile
cp \
    Daemon/Developer_ID_KarabinerVirtualHIDDeviceDaemon.provisionprofile \
    Daemon/build/Release/Karabiner-VirtualHIDDevice-Daemon.app/Contents/embedded.provisionprofile

# Sign
run_with_result_color codesign \
    --sign "$CODE_SIGN_IDENTITY" \
    --entitlements Daemon/entitlements.plist \
    --options runtime \
    --verbose \
    --force \
    Daemon/build/Release/Karabiner-VirtualHIDDevice-Daemon.app
