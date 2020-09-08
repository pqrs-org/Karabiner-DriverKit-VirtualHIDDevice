#!/bin/bash

#
# Deactivate extension
#

/usr/bin/osascript -e 'display dialog "Removing Karabiner VirtualHIDDevice driver.\nThe administrator password will be required to complete." buttons {"OK"}'
# We have to request deactivation from console user.
console_uid=$(stat -f '%u' /dev/console)
/bin/launchctl asuser $console_uid /Applications/.Karabiner-VirtualHIDDevice-Manager.app/Contents/MacOS/Karabiner-VirtualHIDDevice-Manager deactivate

#
# uninstall_core.sh
#

/bin/bash '/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts/uninstall_core.sh'

exit 0
