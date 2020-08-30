#!/bin/bash

/bin/bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall_core.sh'

/usr/bin/osascript -e 'display dialog "Karabiner-DriverKit-VirtualHIDDevice has been uninstalled.\nPlease restart your system." buttons {"OK"}'

exit 0
