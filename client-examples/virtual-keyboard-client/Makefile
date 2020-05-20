CXXFLAGS = -Ivendor/include -std=c++14 -Wall -Werror -O2

all: main.o
	c++ -framework IOKit main.o

clean:
	rm -f *.o
	rm -f a.out

run: all
	./a.out KarabinerDriverKitVirtualHIDKeyboard

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor
