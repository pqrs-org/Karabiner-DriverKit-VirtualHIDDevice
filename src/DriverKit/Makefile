all:
	bash ../scripts/update-version.sh
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"

clean:
	rm -rf build

xcode:
	open KarabinerDriverKitVirtualHIDKeyboard.xcodeproj
