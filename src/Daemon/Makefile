all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"

clean:
	rm -rf *.xcodeproj
	rm -rf build

xcode:
	open *.xcodeproj

codesign:
	$(MAKE) -C .. codesign

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor

run-client:
	sudo ./build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app/Contents/MacOS/Karabiner-DriverKit-VirtualHIDDeviceClient
