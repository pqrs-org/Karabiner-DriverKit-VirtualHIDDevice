all:
	bash scripts/update-info-plist.sh
	xcodegen generate
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"
	$(MAKE) codesign

clean:
	rm -rf KarabinerDriverKitVirtualHIDDevice.xcodeproj
	rm -rf build

xcode:
	open KarabinerDriverKitVirtualHIDDevice.xcodeproj

codesign:
	bash scripts/codesign.sh build/Release

verify:
	codesign -vvv -display build/Release/KarabinerDriverKitVirtualHIDDevice.app
	@echo "\n"
	codesign --display --entitlements :- build/Release/KarabinerDriverKitVirtualHIDDevice.app

run:
	open build/Release/KarabinerDriverKitVirtualHIDDevice.app

systemextensionsctl-list:
	systemextensionsctl list

log-show-extension-manager:
	log show --predicate 'eventMessage CONTAINS "KarabinerDriverKitVirtualHIDDevice"' --info --debug --last 1h

log-stream-extension-manager:
	log stream --predicate 'eventMessage CONTAINS "KarabinerDriverKitVirtualHIDDevice"'
