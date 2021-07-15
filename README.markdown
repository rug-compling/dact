# Decaffeinated Alpino Corpus Tool

## Introduction

Dact is a tool for viewing and analyzing treebanks generated and used by the
[Alpino parser](http://www.let.rug.nl/~vannoord/alp/Alpino/). There is also
experimental support for the TüBa-D/Z treebank of written German.

See the [Dact homepage](http://rug-compling.github.io/dact/) for more information.

## Building

Make sure that you have the following dependencies installed before building Dact:

 * [AlpinoCorpus](http://github.com/rug-compling/alpinocorpus)
 * Meson
 * Qt 5

### Build against system packages

If Xerces-C and XQilla are installed as a system package, Dact can be
built as follows:

```bash
$ meson builddir
$ ninja -C builddir
# If you want to install dact:
$ ninja -C builddir install
```

### Build against DB XML bundle

If Berkeley DB XML, XQilla, Xerces-C, and Berkeley DB are installed as
a bundle using the upstream Berkeley DB XML distribution, there are
two options for building Dact.

Assuming that the DB XML bundle is installed in `/opt/dbxml`, the
first option is to build Dact as follows:

```bash
$ meson builddir -D dbxml_bundle=/opt/dbxml
$ ninja -C builddir
# If you want to install the library:
$ ninja -C builddir install
```

This embeds the DB, Xerces-C, XQilla, and DB XML library paths in the
Dact binary. This will allow you to use Dact without further ado.

The second option is to instead define some variables before building
to point Meson to the headers and libraries:

```bash
$ export LD_LIBRARY_PATH=/opt/dbxml/lib
$ export LIBRARY_PATH=/opt/dbxml/lib
$ export CPATH=/opt/dbxml/include
$ meson builddir
$ ninja -C builddir
# If you want to install the library:
$ ninja -C builddir install
```

This does not embed the DB XML library paths into the Dact binary.
Consequently, `LD_LIBRARY_PATH` should always be set when using Dact
(`LIBRARY_PATH` and `CPATH` only have to be set at build time).

## License

~~~
Copyright 2010-2020 Daniël de Kok
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
