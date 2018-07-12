Acinerella
==========

About
-----

Acinerella is a simple wrapper for the playback facilities of the FFMpeg libraries libavcodec and libavutils.
Because of a very simple API that only needs about 10 functions, it takes only a few minutes to convert its header file.
Therefore Acinerella is easy to use with other programming languages and lets you create your own video or audio player
in a very short time.

Acinerella allows you to load your media streams directly from memory without the need of registering an ominous protocol
format.

Features
--------

**Easy to use API**

Acinerella does all the "FFMpeg stuff" for you. Decoding videos was never easier.

**A single compact library**

FFMpeg can be staticaly linked into Acinerella

**Easy to use with other programming languages**

The Acinerella header file has only about 120 lines and is easy to port to other programming languages. A pascal header exists.

**Cross plattform**

Acinerella and FFMpeg are cross-plattform - A Win32 package is available.

Usage
-----

Make sure you have a recent version of FFMpeg and CMake installed. Under Linux, building Acinerella is as simple as
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
