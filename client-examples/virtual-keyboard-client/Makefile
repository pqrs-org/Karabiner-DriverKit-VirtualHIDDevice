CXXFLAGS = -Ivendor/include -std=c++14 -Wall -Werror -O2

all: main.o
	c++ -framework IOKit main.o
	$(MAKE) codesign

clean:
	rm -f *.o
	rm -f a.out

run: all
	./a.out KarabinerDriverKitVirtualHIDKeyboard

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor

codesign:
	codesign \
		--sign 2772A6CA4AFC07DF97C2BB1BFD92EA182BF4B2B5 \
    --entitlements entitlements.plist \
		--options runtime \
    --verbose \
    --force \
		a.out
