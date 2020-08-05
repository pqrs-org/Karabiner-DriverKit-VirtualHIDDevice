all:
	/usr/bin/python3 ../../../scripts/update-version.py
	xcodegen generate
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"
	$(MAKE) codesign

clean:
	rm -rf Karabiner-DriverKit-VirtualHIDDeviceClient.xcodeproj
	rm -rf build

xcode:
	open Karabiner-DriverKit-VirtualHIDDeviceClient.xcodeproj

codesign:
	bash scripts/codesign.sh

run:
	open build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor
