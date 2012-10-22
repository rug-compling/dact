## Changelog

# 1.7 (not released)

 * Validate queries against the DTD. If a query contains attributes or
   elements that are not allowed by the DTD, query fields are yellow.

# 1.6

 * Recursively open directories of Dact corpora, such as Lassy large.
 * Provide query processing progress information when opening multiple corpora.
 * Correctly bracket discontinuous constituents.
 * Colored matches in keyword in context output.
 * Support query pipelines. For example, *query1 +|+ query2* runs the matches
   of *query1* through *query2*. This is useful when *query2* is very slow,
   and a fast pre-selection query *query1* is available.
 * Full-screen support on Mac OS X.
 * The macro file parser is rewritten, and should give more accurate
   information if the macro file contains errors.
