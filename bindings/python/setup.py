import os

from setuptools import setup, Extension

cmdclass = {}
try:
    import pypandoc
    long_description = pypandoc.convert('README.md', 'rst')
except ImportError:
    long_description = None

if os.getenv('USE_CYTHON', False):
    from Cython.Build import build_ext
    module_src = 'libundo.pyx'
    cmdclass['build_ext'] = build_ext
else:
    module_src = 'libundo.cc'

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
          Extension('libundo', [module_src],
                    include_dirs=[include],
                    depends=[os.path.join(include, 'libundo.h')],
                    language='c++',
                    extra_compile_args=['-O3', '-std=c++14'])
      ],
      cmdclass=cmdclass)
