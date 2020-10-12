#!/bin/bash

# Replace with your identity
readonly CODE_SIGN_IDENTITY=C6DD0BCD24C737EA0505F1EB26B8BBEEDEC12F1B

set -e # forbid command failure

codesign \
    --sign $CODE_SIGN_IDENTITY \
    --options runtime \
    --verbose \
    --force \
    build/Release/HIDManagerTool.app
