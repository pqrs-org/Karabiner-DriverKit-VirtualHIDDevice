all:
	@for d in `find * -type d`; do \
		if [ -f "$$d/CMakeLists.txt" ]; then \
			echo "[Test] $$d"; \
			make -C $$d || exit 1; \
		fi; \
	done

clean:
	@for d in `find * -type d`; do \
		if [ -f "$$d/CMakeLists.txt" ]; then \
			make -C $$d clean; \
		fi; \
	done

update_vendor:
	rm -rf vendor
	cget install pqrs-org/cget-recipes --prefix vendor
	cget install -f cget-requirements.txt --prefix vendor
