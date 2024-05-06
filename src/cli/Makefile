all:
	/usr/bin/python3 ../../scripts/update_version.py
	xcodegen generate
	xcodebuild -configuration Release -alltargets SYMROOT="$(CURDIR)/build"

clean:
	rm -rf *.xcodeproj
	rm -rf build

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor
