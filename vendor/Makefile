all:
	rm -fr vendor
	cmake -S . -B build
	cmake --build build

clean:
	rm -fr build vendor

update:
	git -C cpm-cmake-package-lock pull
	$(MAKE) all
