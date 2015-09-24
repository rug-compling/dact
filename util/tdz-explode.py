#!/usr/bin/python
#
# 'Explode' a TueBa-D/Z ExportXML file to separate files that are ready
# for Dact.
#

from copy import deepcopy

from lxml import etree

import sys

def addWordsSent(elem, begin):
  for nodeElem in elem:
    if nodeElem.tag == 'word':
      nodeElem.set('begin', str(begin))
      nodeElem.set('end', str(begin + 1))
      begin += 1
    else:
      begin = addWordsSent(nodeElem, begin)

  if len(elem) > 0:
    elemBegin = elem[0].get('begin')
    elemEnd = elem[-1].get('end')
    elem.set('begin', str(elemBegin))
    elem.set('end', str(elemEnd))

  return begin

if __name__ == "__main__":
  if len(sys.argv) != 2:
    print("Usage: %s export.xml" % sys.argv[0])
    sys.exit(1)

  exportXML = open(sys.argv[1], 'rb')
  for evt, elem in etree.iterparse(exportXML):
    if elem.tag == 'text':
      textElem = elem
      for sentElem in textElem:
        ident = sentElem.get('{http://www.w3.org/XML/1998/namespace}id')


        with open("%s.xml" % ident, 'wb') as f:
          wrap = etree.Element("tueba_tree")
          wrapNode = etree.Element("node")
          wrapNode.set('cat', 'root')
          wrapNode.set('func', 'root')
          for i in range(len(sentElem)):
            wrapNode.append(deepcopy(sentElem[i]))
          addWordsSent(wrapNode, 0)
          wrap.append(wrapNode)
          f.write(etree.tostring(wrap))
          print("%s\t" % ident)
      elem.clear()
