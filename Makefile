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
	rm -f Karabiner-DriverKit-ExtensionManager-$(VERSION).dmg
	create-dmg \
		--overwrite \
		--identity=$(DMG_IDENTITY) \
		--dmg-title=ExtensionManager \
		src/build/Release/Karabiner-DriverKit-ExtensionManager.app
	mv "Karabiner-DriverKit-ExtensionManager $(VERSION).dmg" dist/Karabiner-DriverKit-ExtensionManager-$(VERSION).dmg

notarize:
	xcrun altool --notarize-app \
		-t osx \
		-f dist/Karabiner-DriverKit-ExtensionManager-$(VERSION).dmg \
		--primary-bundle-id 'org.pqrs.Karabiner-DriverKit-ExtensionManager' \
		-u 'tekezo@pqrs.org' \
		-p '@keychain:pqrs.org-notarize-app'

staple:
		xcrun stapler staple dist/Karabiner-DriverKit-ExtensionManager-$(VERSION).dmg
