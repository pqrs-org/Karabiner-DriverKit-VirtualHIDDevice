#!/bin/bash

set -u # forbid undefined variables
set -e # forbid command failure

# Check Xcode version

xcodePath=$(xcode-select -p | sed 's|\.app/.*|.app|g')
xcodeVersion=$(plutil -extract CFBundleShortVersionString raw "$xcodePath/Contents/version.plist")

if [ "$xcodeVersion" != '13.0' ]; then
    echo
    echo 'ERROR:'
    echo '  Xcode version is not 13.0.'
    echo '  You have to use Xcode 13.0 to support macOS 11 Big Sur.'
    echo
    exit 1
fi

# Package build into a signed .dmg file

version=$(cat version)

make build

# --------------------------------------------------
echo "Copy Files"

rm -rf pkgroot
mkdir -p pkgroot

basedir="pkgroot/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts"
mkdir -p "$basedir"
cp -R files/scripts/uninstall "$basedir"

basedir="pkgroot/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/Applications"
mkdir -p "$basedir"
cp -R "src/Client/build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app" "$basedir"

basedir="pkgroot/Library"
mkdir -p "$basedir"
cp -R files/LaunchDaemons "pkgroot/Library"

basedir="pkgroot/Applications"
mkdir -p "$basedir"
# Note: Rename app (add leading dot) in order to hide from Finder and Launchpad.
cp -R "src/Manager/build/Release/Karabiner-VirtualHIDDevice-Manager.app" "$basedir/.Karabiner-VirtualHIDDevice-Manager.app"

bash "scripts/setpermissions.sh" pkginfo
bash "scripts/setpermissions.sh" pkgroot

chmod 755 pkginfo/Scripts/postinstall
chmod 755 pkginfo/Scripts/preinstall

# --------------------------------------------------
echo "Create pkg"

pkgName="Karabiner-DriverKit-VirtualHIDDevice-$version.pkg"
pkgIdentifier="org.pqrs.Karabiner-DriverKit-VirtualHIDDevice"

rm -f dist/$pkgName

pkgbuild \
    --root pkgroot \
    --component-plist pkginfo/pkgbuild.plist \
    --scripts pkginfo/Scripts \
    --identifier $pkgIdentifier \
    --version $version \
    --install-location "/" \
    dist/$pkgName

# --------------------------------------------------
echo "Sign with Developer ID"

set +e # allow command failure
bash scripts/codesign-pkg.sh dist/$pkgName
