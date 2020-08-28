all:
	/usr/bin/python3 ../scripts/update-version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"
	$(MAKE) -C DriverKit
	mkdir -p build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions
	rsync -a --delete \
		DriverKit/build/Release-driverkit/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext \
		build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions
	$(MAKE) codesign

clean:
	rm -rf *.xcodeproj
	rm -rf build
	$(MAKE) -C DriverKit clean

xcode:
	open Karabiner-DriverKit-VirtualHIDDevice.xcodeproj

codesign:
	bash scripts/codesign.sh

verify:
	2>&1 codesign -dvvv build/Release/Karabiner-DriverKit-ExtensionManager.app
	@echo "\n"
	2>&1 codesign -d --entitlements :- build/Release/Karabiner-DriverKit-ExtensionManager.app
	@echo "\n"
	2>&1 codesign -dvvv build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext
	@echo "\n"
	2>&1 codesign -d --entitlements :- build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext
	@echo "\n"
	2>&1 codesign -dvvv build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app
	@echo "\n"
	2>&1 codesign -d --entitlements :- build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor

log-show:
	log show --predicate 'sender == "sysextd" or sender CONTAINS "org.pqrs"' --info --debug --last 1h

run-client:
	sudo /bin/launchctl asuser 0 ./build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app/Contents/MacOS/Karabiner-DriverKit-VirtualHIDDeviceClient
