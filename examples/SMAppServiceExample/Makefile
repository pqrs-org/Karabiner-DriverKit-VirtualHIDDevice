all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"
# Copy LaunchDaemons
	mkdir -p build/Release/Karabiner-VirtualHIDDevice-SMAppServiceExample.app/Contents/Library/LaunchDaemons
	cp ../../files/LaunchDaemons/org.pqrs.Karabiner-VirtualHIDDevice-Daemon.plist build/Release/Karabiner-VirtualHIDDevice-SMAppServiceExample.app/Contents/Library/LaunchDaemons

clean:
	rm -rf *.xcodeproj
	rm -rf build

xcode:
	open *.xcodeproj
