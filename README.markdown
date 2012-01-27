# Introduction

Dact is a tool for viewing and analyzing treebanks generated and used by
the [Alpino parser](http://www.let.rug.nl/~vannoord/alp/Alpino/).

See the [Dact homepage](http://rug-compling.github.com/dact/) for more information.

# Building

Make sure that you have the following dependencies installed before building Dact:

 * [AlpinoCorpus](http://github.com/rug-compling/alpinocorpus)
 * CMake
 * Qt 4.7

If you want to build the current stable/release version of Dact, check out the
*release* branch first. Then build Dact with:

    cmake .
    make
    make install

# Limitations

When a corpus reader with XPath 1.0 is used, the statistics tab does not
provide a count of matching nodes that do not have the given attribute.
