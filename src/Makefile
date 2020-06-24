all:
	bash ../scripts/update-version.sh
	xcodegen generate
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"
	$(MAKE) -C DriverKit
	mkdir -p build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions
	rsync -a --delete \
		DriverKit/build/Release-driverkit/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext \
		build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions
	$(MAKE) codesign

clean:
	rm -rf Karabiner-DriverKit-ExtensionManager.xcodeproj
	rm -rf build
	$(MAKE) -C DriverKit clean

xcode:
	open Karabiner-DriverKit-ExtensionManager.xcodeproj

codesign:
	bash scripts/codesign.sh

verify:
	codesign -vvv -display build/Release/Karabiner-DriverKit-ExtensionManager.app
	@echo "\n"
	codesign --display --entitlements :- build/Release/Karabiner-DriverKit-ExtensionManager.app
	@echo "\n"
	codesign -vvv -display build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext
	@echo "\n"
	codesign --display --entitlements :- build/Release/Karabiner-DriverKit-ExtensionManager.app/Contents/Library/SystemExtensions/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice.dext

log-show-extension-manager:
	log show --predicate 'sender == "sysextd" or sender CONTAINS "org.pqrs"' --info --debug --last 1h
