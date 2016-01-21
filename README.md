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

* **Easy to use API**
  Acinerella does all the "FFMpeg stuff" for you. Decoding videos was never easier.
* **A single compact library**
  FFMpeg can be staticaly linked into Acinerella
* **Easy to use with other programming languages**
  The Acinerella header file has only about 120 lines and is easy to port to other programming languages. A pascal header exists.
* **Cross plattform**
  Acinerella and FFMpeg are cross-plattform - A Win32 package is available.

Usage
-----

Under Linux, simply run the following commands to compile Acinerella:

```bash
git clone https://github.com/astoeckel/acinerella
cd acinerella && mkdir build && cd build;
cmake .. && make
```

License
-------

Copyright (C) 2008-2016 Andreas Stoeckel

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
http://www.gnu.org/licenses/
