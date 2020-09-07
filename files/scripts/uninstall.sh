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

exit 0
