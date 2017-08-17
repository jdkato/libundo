import codecs
import os

from setuptools import setup, Extension

cmdclass = {}
long_description = ''

USE_CYTHON = os.getenv('USE_CYTHON', False)
if USE_CYTHON:
    from Cython.Build import build_ext
    module_src = 'libundo.pyx'
    cmdclass['build_ext'] = build_ext
else:
    module_src = 'libundo.cc'

# Load README into long description.
here = os.path.abspath(os.path.dirname(__file__))
with codecs.open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

src = os.path.join('..', '..', 'src')
include = os.path.join('..', '..', 'include')
setup(name='libundo',
      description='Branching, persistent undo/redo.',
      long_description=long_description,
      version='0.1.0',
      url='https://github.com/libundo/libundo',
      author='Joseph Kato',
      author_email='joseph@jdkato.io',
      license='Apache-2.0',
      ext_modules=[
          Extension('libundo', [module_src, os.path.join(src, 'libundo.cc')],
                    include_dirs=[include],
                    depends=[os.path.join(include, 'libundo.h')],
                    language='c++',
                    extra_compile_args=['-O3', '-std=c++11'])
      ],
      cmdclass=cmdclass)
