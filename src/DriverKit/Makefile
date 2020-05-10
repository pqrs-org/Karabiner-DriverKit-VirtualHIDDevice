all:
	bash ../scripts/update-info-plist.sh
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"

clean:
	rm -rf build

xcode:
	open KarabinerDriverKitVirtualHIDKeyboard.xcodeproj
