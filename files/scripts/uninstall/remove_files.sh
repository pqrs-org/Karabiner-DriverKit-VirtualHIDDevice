#!/bin/bash

#
# Note: This script must be run with root account.
#

PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

#
# Unload files which are installed in previous versions
#

if [ -f /Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist ]; then
    launchctl bootout system /Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist
fi

#
# Kill processes in previous versions
#

killall Karabiner-DriverKit-VirtualHIDDeviceClient

#
# Remove files
#

rm -rf '/Applications/.Karabiner-VirtualHIDDevice-Manager.app'
rm -rf '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice'

rm -f '/Library/Application Support/org.pqrs/tmp/rootonly/virtual_hid_device_service_server.*'
rm -rf '/Library/Application Support/org.pqrs/tmp/rootonly/vhidd_server'
rmdir '/Library/Application Support/org.pqrs/tmp/rootonly'
rmdir '/Library/Application Support/org.pqrs/tmp'
rmdir '/Library/Application Support/org.pqrs'

#
# Remove files which are installed in previous versions
#

rm -f '/Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient.plist'

exit 0
