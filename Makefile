all:
	$(MAKE) -C src
	$(MAKE) -C client/examples

clean:
	$(MAKE) -C src clean
	$(MAKE) -C client/examples clean
	$(MAKE) -C tests clean
