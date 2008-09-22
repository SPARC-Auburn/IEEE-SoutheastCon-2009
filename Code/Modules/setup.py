from distutils.core import setup, Extension

setup(name="laserutil", version="pre 1.0",
	ext_modules=[Extension("laserutil", ["laserutilmodule.c"])])