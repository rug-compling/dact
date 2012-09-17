## Changelog

# 1.6 (not released)

 * Recursively open directories of Dact corpora.
 * Provide query processing progress information when opening multiple corpora.
 * Correctly bracket discontiuous constituents.
 * Colored matches in keyword in context output.
 * Support query pipelines. For example, *query1 +|+ query2* runs the matches
   of *query1* through *query2*. This is useful when *query2* is very slow,
   and a fast preselection query *query1* is available.
 * Full-screen support on Mac OS X.
 * The macro file parser is rewritten, and should give more accuracte
   information then the macro file contains errors.