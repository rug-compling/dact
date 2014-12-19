#!/usr/local/bin/python
# -*- coding: UTF-8 -*-

from lxml import etree

import sys

def createGraph(tokens, n):
  # Add the root
  root = etree.Element("node")
  tokenElements = [root]
  root.set('rel', 'root')

  for token in tokens:
    tokenElem = etree.Element("node")
    tokenElem.set('word', token[1].decode('utf-8'))
    tokenElem.set('lemma', token[2].decode('utf-8'))
    tokenElem.set('cpos', token[3])
    tokenElem.set('pos', token[4])
    tokenElem.set('morph', token[5])
    tokenElem.set('rel', token[7])
    tokenElements.append(tokenElem)

  # Connect graph
  for i in range(len(tokens)):
    token = tokens[i]
    tokenElem = tokenElements[i + 1]
    tokenElem.set('begin', str(i + 1))
    tokenElem.set('end', str(i + 2))
    headToken = int(token[6])
    tokenElements[headToken].append(tokenElements[i + 1])

  wrap = etree.Element("simple_ds")
  wrap.append(root)

  with open("s%d.xml" % n, 'w') as f:
    f.write(etree.tostring(wrap, pretty_print = True))

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print "Usage: %s export.xml" % sys.argv[0]
    sys.exit(1)

  with open(sys.argv[1], 'r') as conllInput:
    tokens = []
    n = 1
    for line in conllInput:
      line = line.strip()

      if not line:
        createGraph(tokens, n)
        tokens = []
        n += 1
      else:
        lineParts = line.split('\t')
        tokens.append(lineParts)
