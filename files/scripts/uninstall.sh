#!/bin/bash

#
# Deactivate extension
#

/usr/bin/osascript -e 'display dialog "Removing Karabiner VirtualHIDDevice driver.\nThe administrator password will be required to complete." buttons {"OK"}'
/Applications/.Karabiner-VirtualHIDDevice-Manager.app/Contents/MacOS/Karabiner-VirtualHIDDevice-Manager deactivate

#
# uninstall_core.sh
#

/bin/bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall_core.sh'

/usr/bin/osascript -e 'display dialog "Karabiner-DriverKit-VirtualHIDDevice has been uninstalled.\nPlease restart your system." buttons {"OK"}'

exit 0
