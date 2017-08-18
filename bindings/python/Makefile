default: build

FILES=libundo.cc setup.py MANIFEST.in README.md

lib: libundo.pyx ctree.pxd
	cython --cplus libundo.pyx -o libundo.cc

build: ${FILES}
	python setup.py build_ext -i

sdist: ${FILES}
	python setup.py sdist

publish: clean sdist
	twine upload dist/*

clean:
	rm -rf libundo dist libundo.egg-info build
	rm -f libundo.cc libundo.*.so
