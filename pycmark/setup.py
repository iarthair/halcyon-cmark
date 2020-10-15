from setuptools import setup, Extension

#with open("README.md", "r") as fh:
#    long_description = fh.read()

setup(
    name='halcyon_cmark',
    version='1.0.1',
    author='Brian Stafford',
    author_email='brian.stafford60+halcyon@gmail.com',
    description='Static website builder.',
    ext_modules=[
        Extension('cmark', sources=['module.c',],
                           extra_compile_args=['-std=c99', '-O2'],
                           libraries=['cmark-gfm','cmark-gfm-extensions',]),

    ],
#    long_description=long_description,
#    long_description_content_type="text/markdown",
    url='https://iarthair.github.io/',
#    include_package_data=False,
#    install_requires=[
#        'pyyaml',
#        'pymmd',
#        'jinja2',
#    ],
#    classifiers=[
#        "Programming Language :: Python :: 3",
#        "License :: OSI Approved :: GNU General Public License (GPL)",
#        "Operating System :: OS Independent",
#    ],
    python_requires='>=3.6',
)
