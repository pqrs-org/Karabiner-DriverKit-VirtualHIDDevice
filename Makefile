VERSION = `head -n 1 version`
DMG_IDENTITY = 'Developer ID Application: Fumihiko Takayama (G43BCU2T37)'

all:
	$(MAKE) -C src
	$(MAKE) -C client/examples

clean:
	$(MAKE) -C src clean
	$(MAKE) -C client/examples clean
	$(MAKE) -C tests clean

package:
	$(MAKE) -C src
	$(MAKE) -C client/examples

	rm -f Karabiner-VirtualHIDDevice-Manager-$(VERSION).dmg
	create-dmg \
		--overwrite \
		--identity=$(DMG_IDENTITY) \
		--dmg-title=VirtualHIDDevice-Manager \
		src/build/Release/Karabiner-VirtualHIDDevice-Manager.app
	mv "Karabiner-VirtualHIDDevice-Manager $(VERSION).dmg" dist/Karabiner-VirtualHIDDevice-Manager-$(VERSION).dmg

	rm -f Karabiner-DriverKit-VirtualHIDDeviceClient-$(VERSION).dmg
	create-dmg \
		--overwrite \
		--identity=$(DMG_IDENTITY) \
		--dmg-title=VirtualHIDDeviceClient \
		client/examples/virtual-keyboard-client/build/Release/Karabiner-DriverKit-VirtualHIDDeviceClient.app
	mv "Karabiner-DriverKit-VirtualHIDDeviceClient $(VERSION).dmg" dist/Karabiner-DriverKit-VirtualHIDDeviceClient-$(VERSION).dmg

notarize:
	xcrun altool --notarize-app \
		-t osx \
		-f dist/Karabiner-VirtualHIDDevice-Manager-$(VERSION).dmg \
		--primary-bundle-id 'org.pqrs.Karabiner-VirtualHIDDevice-Manager' \
		-u 'tekezo@pqrs.org' \
		-p '@keychain:pqrs.org-notarize-app'

	xcrun altool --notarize-app \
		-t osx \
		-f dist/Karabiner-DriverKit-VirtualHIDDeviceClient-$(VERSION).dmg \
		--primary-bundle-id 'org.pqrs.Karabiner-DriverKit-VirtualHIDDeviceClient' \
		-u 'tekezo@pqrs.org' \
		-p '@keychain:pqrs.org-notarize-app'

staple:
		xcrun stapler staple dist/Karabiner-VirtualHIDDevice-Manager-$(VERSION).dmg
		xcrun stapler staple dist/Karabiner-DriverKit-VirtualHIDDeviceClient-$(VERSION).dmg
