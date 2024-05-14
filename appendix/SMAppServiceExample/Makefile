all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"
# Copy LaunchDaemons
	mkdir -p build/Release/Karabiner-DriverKit-SMAppServiceExample.app/Contents/Library
	cp -r LaunchDaemons build/Release/Karabiner-DriverKit-SMAppServiceExample.app/Contents/Library
	$(MAKE) codesign

clean:
	rm -rf *.xcodeproj
	rm -rf build

xcode:
	open *.xcodeproj

codesign:
	bash scripts/codesign.sh
