#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=BD3B995B69EBA8FC153B167F063079D19CCC2834

set -e # forbid command failure

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --options runtime \
    --verbose \
    --force \
    build/Release/Karabiner-DriverKit-SMAppServiceExample.app
