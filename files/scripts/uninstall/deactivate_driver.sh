#!/bin/bash

#
# Note: This script must be run with console user account.
#

PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

osascript -e 'display dialog "Removing Karabiner VirtualHIDDevice driver.\nThe administrator password will be required to complete." buttons {"OK"}'
/Applications/.Karabiner-VirtualHIDDevice-Manager.app/Contents/MacOS/Karabiner-VirtualHIDDevice-Manager deactivate
