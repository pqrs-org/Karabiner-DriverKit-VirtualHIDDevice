VERSION = `/usr/bin/python3 scripts/get_version.py package_version`

CLANG_FORMAT_FILES = \
	'examples/*.cpp' \
	'include/*.hpp' \
	'include/*.hpp.in' \
	'src/*.cpp' \
	'src/*.hpp.in' \
	'src/*.hpp' \
	'src/*.iig' \
	'tests/*.cpp' \
	'tests/*.hpp'

all:
	@echo 'Type `make package`'

package: clean
	bash make-package.sh
	$(MAKE) clean-launch-services-database

build:
	$(MAKE) -C src

clean:
	git clean -x -f -d *

clean-launch-services-database:
	$(MAKE) -C tools/clean-launch-services-database

notarize:
	xcrun notarytool \
		submit dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).pkg \
		--keychain-profile "pqrs.org notarization" \
		--wait
	$(MAKE) staple
	say "notarization completed"

staple:
	xcrun stapler staple dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).pkg

check-staple:
	@xcrun stapler validate `find dist | sort -V | tail -n 1`

format: clang-format swift-format

clang-format:
	git ls-files -z -- $(CLANG_FORMAT_FILES) | xargs -0 clang-format -i

swift-format:
	find * -name '*.swift' -print0 | xargs -0 swift-format -i

swiftlint:
	swiftlint

notarized-pkg: package notarize
