# Dact dependencies on a treebank
- xml elements named 'node'

## tree.xsl
- /alpino_ds/node
- @rel
- @pb (optional)
- @index
- @cat | @pt
- @pos (optional)
- @word (optional)
- @postag | @pos (optional)

## bracketed-sentence-xml.xsl
- sentence element with words separated by spaces in root element.
- node elements
- @begin
- @end
- @cat | @root

# Alpinocorpus dependencies
Alpinocorpus::getSentence depends on `collectLexicals`, which selects nodes using `//node[@word]` and queries their `@begin` property.
