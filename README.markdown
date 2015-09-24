# Decaffeinated Alpino Corpus Tool

## Introduction

Dact is a tool for viewing and analyzing treebanks generated and used by the
[Alpino parser](http://www.let.rug.nl/~vannoord/alp/Alpino/). There is also
experimental support for the TüBa-D/Z treebank of written German.

See the [Dact homepage](http://rug-compling.github.com/dact/) for more information.

## Building

Make sure that you have the following dependencies installed before building Dact:

 * [AlpinoCorpus](http://github.com/rug-compling/alpinocorpus)
 * CMake
 * Qt 4.8

If you want to build the current stable/release version of Dact, check out the
*release* branch first. Then build Dact with:

    cmake .
    make
    make install

If you are using Homebrew on Mac OS X, you can simply compile Dact with:

    brew tap rug-compling/dact
    brew install dact

This will fetch, compile, and install Dact and its dependencies.

## License

~~~
Copyright 2010-2014 Daniël de Kok
Copyright 2010-2012 University of Groningen

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the 
Free Software Foundation, Inc.,  51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301  USA
~~~
