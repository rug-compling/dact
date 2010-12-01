---
layout: base
title: Installation
---

## Windows

Recent Windows versions can handle zip-archives. In such case, double-click
the Dact archive, and drag the folder to you preferred location. If your Windows
installation cannot open zip-archives, you can download a freely available
archiver such as [7-Zip](http://www.7-zip.org/).

### Notes

If you get the error *The application failed to initialize properly
(0xc0000022)* it means that something is wrong with the DLL permissions.
Presumably with the *.dll files in the Dact distribution.

You can fix this problem by opening a command prompt, and *cd* to the
directory in which the Dact files reside. Then type:

    cacls *.dll /E /G BUILTIN\Users:R 

## Mac OS X

Download a diskimage (.dmg) file from the Dact download site. After
downloading, open the image by double-clicking, and drag Dact to the
*Applications* folder. Afterward, you can safely eject the Dact disk image.

## Compiling Dact from source

**Warning:** if you are not familiar with compiling C/C++ programs,
downloading a pre-built Dact version is highly recommended.

Dact and its dependencies (*qdictzip*, *alpinocorpus*) can be compiled
using [cmake](http://www.cmake.org/). For each source distribution,
execute the following commands:

    cmake .
    make
    make install
