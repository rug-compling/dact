#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import argparse

from lxml import etree

def extract_features(tokenElem, feature_str):
    if feature_str == "_":
        return

    for fv in feature_str.split('|'):
        fv = fv.split(':')
        if len(fv) == 1:
            tokenElem.set(fv[0], "1")
        elif len(fv) == 2:
            tokenElem.set(fv[0], fv[1])

def createGraph(tokens, n, explode_features):
  # Add the root
  root = etree.Element("word")
  tokenElements = [root]
  root.set('rel', 'root')

  for token in tokens:
    tokenElem = etree.Element("word")
    tokenElem.set('form', token[1])
    tokenElem.set('lemma', token[2])
    tokenElem.set('cpos', token[3])
    tokenElem.set('pos', token[4])
    tokenElem.set('rel', token[7])

    if explode_features:
      extract_features(tokenElem, token[5])
    else:
      tokenElem.set('features', token[5])

    tokenElements.append(tokenElem)

  # Connect graph
  for i in range(len(tokens)):
    token = tokens[i]
    tokenElem = tokenElements[i + 1]
    tokenElem.set('begin', str(i + 1))
    tokenElem.set('end', str(i + 2))
    headToken = int(token[6])
    tokenElements[headToken].append(tokenElements[i + 1])

  wrap = etree.Element("conllx_ds")
  wrap.append(root)

  with open("s%d.xml" % n, 'wb') as f:
    f.write(etree.tostring(wrap, pretty_print = True))

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("conllx", help="CoNLL-X file to convert")
  parser.add_argument("-f", "--features", help="explode features field",
    action="store_true")
  args = parser.parse_args()

  with open(args.conllx, 'r') as conllInput:
    tokens = []
    n = 1
    for line in conllInput:
      line = line.strip()

      if not line:
        createGraph(tokens, n, args.features)
        tokens = []
        n += 1
      else:
        lineParts = line.split('\t')
        tokens.append(lineParts)
