all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"
# Copy LaunchDaemons
	mkdir -p build/Release/Karabiner-VirtualHIDDevice-SMAppServiceExample.app/Contents/Library/LaunchDaemons
	cp ../../files/LaunchDaemons/*.plist build/Release/Karabiner-VirtualHIDDevice-SMAppServiceExample.app/Contents/Library/LaunchDaemons
# codesign
	$(MAKE) codesign

clean:
	rm -rf *.xcodeproj
	rm -rf build

xcode:
	open *.xcodeproj

codesign:
	bash scripts/codesign.sh
