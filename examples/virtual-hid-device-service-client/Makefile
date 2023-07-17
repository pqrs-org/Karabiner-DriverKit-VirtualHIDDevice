all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"

clean:
	rm -rf virtual-hid-device-service-client.xcodeproj
	rm -rf build

run:
	sudo ./build/Release/virtual-hid-device-service-client

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor
