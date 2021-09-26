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
	xcrun altool --notarize-app \
		-t osx \
		-f dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).pkg \
		--primary-bundle-id 'org.pqrs.Karabiner-DriverKit-VirtualHIDDevice' \
		-u 'tekezo@pqrs.org' \
		-p '@keychain:pqrs.org-notarize-app'

staple:
	xcrun stapler staple dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).pkg

check-staple:
	for f in dist/*.pkg; do xcrun stapler validate $$f || exit 1; done

update_vendor:
	for f in $$(find * -name 'cget-requirements.txt'); do make -C $$(dirname $$f) update_vendor; done
