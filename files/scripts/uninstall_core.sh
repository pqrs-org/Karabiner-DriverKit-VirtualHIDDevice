#!/bin/bash

PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

#
# Unload before install
#

if [ /Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist ]; then
    launchctl bootout system /Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist
fi

#
# Kill processes
#

killall Karabiner-DriverKit-VirtualHIDDeviceClient

#
# Uninstall
#

rm -f '/Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist'
rm -rf '/Applications/.Karabiner-VirtualHIDDevice-Manager.app'
rm -rf '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice'

exit 0
