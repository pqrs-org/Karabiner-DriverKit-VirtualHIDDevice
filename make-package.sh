#!/bin/bash

set -u # forbid undefined variables
set -e # forbid command failure

# Package build into a signed .dmg file

version=$(cat version)

make build

# --------------------------------------------------
echo "Copy Files"

rm -rf pkgroot
mkdir -p pkgroot

basedir="pkgroot/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/scripts"
mkdir -p "$basedir"
cp files/scripts/uninstall.sh "$basedir"
cp files/scripts/uninstall_core.sh "$basedir"
cp files/scripts/uninstaller.applescript "$basedir"

basedir="pkgroot/Library/Application Support/org.pqrs/Karabiner-DriverKit-VirtualHIDDevice/Applications"
mkdir -p "$basedir"
cp -R "src/build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app" "$basedir"

basedir="pkgroot/Library"
mkdir -p "$basedir"
cp -R files/LaunchDaemons "pkgroot/Library"

basedir="pkgroot/Applications"
mkdir -p "$basedir"
# Note: Rename app (add leading dot) in order to hide from Finder and Launchpad.
cp -R "src/build/Release/Karabiner-VirtualHIDDevice-Manager.app" "$basedir/.Karabiner-VirtualHIDDevice-Manager.app"

bash "scripts/setpermissions.sh" pkginfo
bash "scripts/setpermissions.sh" pkgroot

chmod 755 pkginfo/Scripts/postinstall
chmod 755 pkginfo/Scripts/preinstall

# --------------------------------------------------
echo "Create pkg"

pkgName="Karabiner-DriverKit-VirtualHIDDevice.pkg"
pkgIdentifier="org.pqrs.Karabiner-DriverKit-VirtualHIDDevice"
archiveName="Karabiner-DriverKit-VirtualHIDDevice-${version}"

rm -rf $archiveName
mkdir $archiveName

pkgbuild \
    --root pkgroot \
    --component-plist pkginfo/pkgbuild.plist \
    --scripts pkginfo/Scripts \
    --identifier $pkgIdentifier \
    --version $version \
    --install-location "/" \
    $archiveName/Installer.pkg

productbuild \
    --distribution pkginfo/Distribution.xml \
    --package-path $archiveName \
    $archiveName/$pkgName

rm -f $archiveName/Installer.pkg

# --------------------------------------------------
echo "Sign with Developer ID"

set +e # allow command failure
bash scripts/codesign-pkg.sh $archiveName/$pkgName

# --------------------------------------------------
echo "Make Archive"

set -e # forbid command failure

# Note:
# Some third vendor archiver fails to extract zip archive.
# Therefore, we use dmg instead of zip.

rm -f dist/$archiveName.dmg
hdiutil create -nospotlight dist/$archiveName.dmg -srcfolder $archiveName -fs 'APFS'
rm -rf $archiveName
chmod 644 dist/$archiveName.dmg
