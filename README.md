Acinerella
==========

About
-----

Acinerella is a simple wrapper for the playback facilities of the [FFmpeg](https://ffmpeg.org) libraries. It features a relatively simple and stable C API with about ten functions. Correspondingly, it should only take a few minutes to translate `acinerella.h` to other programming languages. Hence, Acinerella should be easy to integrate with other programming languages than C, facilitating media decoding in your own application.

Acinerella allows you to load your media streams directly from memory without the need of registering an ominous protocol
format.

Features
--------

**Easy to use API**

Acinerella does all the "FFmpeg stuff" for you. Decoding videos was never easier.

**A single compact library**

FFmpeg can be staticaly linked into Acinerella

**Easy to use with other programming languages**

The Acinerella header file has only about 120 lines and is easy to port to other programming languages. A Pascal header exists.

Usage
-----

Make sure you have a recent version of FFmpeg and CMake installed. Under Linux, building Acinerella is as simple as
runing the following commands:

```bash
git clone https://github.com/astoeckel/acinerella
cd acinerella && mkdir build && cd build;
cmake .. && make
```

Old versions
------------

Unsupported old binary versions of Acinerella may be found at

https://sourceforge.net/projects/acinerella/files/

License
-------

Copyright (C) 2008-2018 Andreas Stoeckel


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
