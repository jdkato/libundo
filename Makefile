.PHONY: py cpp

default: cpp py

py:
	cd bindings/python && \
	make lib && make && \
	python test.py

cpp:
	cd test && \
	cmake . && make && \
	./bin/run_tests
