from distutils.core import setup, Extension

setup(name="sio", version="1.0",
      ext_modules=[Extension("sio", ["siomodule.c"])])
