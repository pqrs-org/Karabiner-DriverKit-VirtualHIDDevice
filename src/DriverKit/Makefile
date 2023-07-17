all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"

clean:
	rm -rf build

xcode:
	open Karabiner-DriverKit-VirtualHIDDevice.xcodeproj
