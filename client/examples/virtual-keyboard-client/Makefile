CXXFLAGS = -Ivendor/include -I../../include -I../../../include -std=c++17 -Wall -Werror -O2

all:
	bash ../../../scripts/update-version.sh
	xcodegen generate
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"
	$(MAKE) codesign

clean:
	rm -rf Karabiner-DriverKit-VirtualHIDKeyboard-Client.xcodeproj
	rm -rf build

xcode:
	open Karabiner-DriverKit-VirtualHIDKeyboard-Client.xcodeproj

codesign:
	bash scripts/codesign.sh

run:
	open build/Release/Karabiner-DriverKit-VirtualHIDKeyboard-Client.app

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor

codesign:
	bash scripts/codesign.sh
