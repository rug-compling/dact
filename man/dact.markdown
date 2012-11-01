% DACT(1)
% Daniel de Kok and Jelmer van der Linde
% Nov 1, 2012
NAME
====

dact -- Alpino treebank viewer

SYNOPSIS
========

dact [*options*] [*treebank ...*]

DESCRIPTION
===========

Dact is a tool for viewing and analyzing Alpino corpora. With Dact you can
open and query Dact corpora, compact corpora and directories with XML files.
Dact can show trees and count occurrences for almost anything you can query
using XPath2 expressions.

If **dact** is invoked without without any arguments, it will show a dialog
allowing you to select a treebank. If a treebank is provided as an argument,
Dact will immediately open that treebank.

The following option is available:

`-m` *MACROFILE*
:    Load macros from the provided macro file.

SEE ALSO
========

For more information, please refer to these online resources:

 * Manual: http://rug-compling.github.com/dact/manual/
 * Cookbook: http://rug-compling.github.com/dact/cookbook/
