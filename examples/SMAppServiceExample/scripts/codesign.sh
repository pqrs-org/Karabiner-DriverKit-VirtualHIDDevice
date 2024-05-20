#!/bin/bash

set -u # forbid undefined variables
set -e # forbid command failure

readonly PATH=/bin:/sbin:/usr/bin:/usr/sbin
export PATH

trap "echo -ne '\033[0m'" EXIT
echo -ne '\033[33;40m'

readonly CODE_SIGN_IDENTITY=$(bash $(dirname $0)/../../../scripts/get-codesign-identity.sh)

if [[ -z $CODE_SIGN_IDENTITY ]]; then
    echo "Skip codesign"
    exit 0
fi

#
# Sign Karabiner-VirtualHIDDevice-SMAppServiceExample
#

# Sign
codesign \
    --sign $CODE_SIGN_IDENTITY \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-VirtualHIDDevice-SMAppServiceExample.app
