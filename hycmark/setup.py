from setuptools import setup, Extension

setup(
    name='halcyon_cmark',
    version='1.0.0',
    author='Brian Stafford',
    author_email='brian.stafford60+halcyon@gmail.com',
    description='Python interface for cmark-gfm',
    ext_modules=[
        Extension('hycmark', sources=['module.c','header.c'],
                             extra_compile_args=['-std=c99','-O2'],
                             libraries=['cmark-gfm','cmark-gfm-extensions',]),

    ],
    url='https://iarthair.github.io/',
    python_requires='>=3.6',
)
