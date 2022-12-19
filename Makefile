VERSION = `head -n 1 version`

all:
	@echo 'Type `make package`'

build:
	$(MAKE) -C src

clean:
	git clean -x -f -d *

package: clean
	bash make-package.sh

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

update_vendor:
	for f in $$(find * -name 'cget-requirements.txt'); do make -C $$(dirname $$f) update_vendor; done

swift-format:
	find * -name '*.swift' -print0 | xargs -0 swift-format -i
