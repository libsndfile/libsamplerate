<img src="http://www.mega-nerd.com/SRC/SRC.png" width="350"></img>

This is libsamplerate, `0.1.9`.

libsamplerate (also known as Secret Rabbit Code) is a library for performing sample rate conversion of audio data.

* The [`src/`](https://github.com/erikd/libsamplerate/tree/master/src) directory contains the source code for library itself.
* The [`doc/`](https://github.com/erikd/libsamplerate/tree/master/doc) directory contains the libsamplerate documentation.
* The [`examples/`](https://github.com/erikd/libsamplerate/tree/master/examples) directory contains examples of how to write code using libsamplerate.
* The [`tests/`](https://github.com/erikd/libsamplerate/tree/master/tests) directory contains programs which link against libsamplerate and test its functionality.
* The [`Win32/`](https://github.com/erikd/libsamplerate/tree/master/Win32) directory contains files and documentation to allow libsamplerate to compile under Win32 with the Microsoft Visual C++ compiler.

Additional references:

* [Official website](http://www.mega-nerd.com/libsamplerate/)
* [GitHub](https://github.com/erikd/libsamplerate)

---

## Build Status

| Branch         | Status                                                                                                            |
|----------------|-------------------------------------------------------------------------------------------------------------------|
| `master`       | [![Build Status](https://travis-ci.org/erikd/libsamplerate.svg?branch=master)](https://travis-ci.org/erikd/libsamplerate)       |

Branches [actively built](https://travis-ci.org/erikd/libsamplerate/branches) by TravisCI.

---

## Win32

There are detailed instructions for building libsamplerate on Win32 in the file [`doc/win32.html`](https://github.com/erikd/libsamplerate/blob/master/doc/win32.html).

## macOS

Building on macOS should be the same as building it on any other Unix platform.

## Other Platforms

To compile libsamplerate on platforms which have a Bourne compatible shell, an ANSI C compiler and a make utility should require no more that the following three commands:
```bash
./configure
make
make install
```

## CMake

There is a new [CMake](https://cmake.org/download/)-based build system available:
```bash
mkdir build
cd build
cmake ..
make
```

* Use `cmake -DCMAKE_BUILD_TYPE=Release ..` to make a release build.
* Use `cmake -DBUILD_SHARED_LIBS=ON ..` to build a shared library.

## Contacts

libsamplerate was written by [Erik de Castro Lopo](mailto:erikd@mega-nerd.com).
