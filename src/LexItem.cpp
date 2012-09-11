#include <algorithm>
#include <cstring>
#include <list>
#include <stdexcept>

#include <QHash>
#include <QString>
#include <QStringList>
#include <QtDebug>

extern "C" {
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
}

#include "LexItem.hh"
#include "parseString.hh"

namespace {
xmlChar const *toXmlStr(char const *str)
{
    return reinterpret_cast<xmlChar const *>(str);
}

char const *fromXmlStr(xmlChar const *str)
{
    return reinterpret_cast<char const *>(str);
}

}

// The function adds one to the count of lexical nodes that are dominated
// by the given node. We could modify the DOM tree directly to store such
// counts in the lexical nodes. But frankly, manipulating the DOM tree is
// a drag. Since we do not modify the tree, we can keep the counts by
// memory adres. Ugly, but effective. Don't we love that?
void LexItem::markLexicals(xmlNode *node, QHash<xmlNode *, QSet<size_t> > *matchDepth,
    size_t matchId)
{
    // Don't attempt to handle a node that we can't.
    if (node->type != XML_ELEMENT_NODE ||
          std::strcmp(fromXmlStr(node->name), "node") != 0)
        return;

    xmlAttrPtr wordProp = xmlHasProp(node, toXmlStr("word"));
    if (wordProp != 0)
        (*matchDepth)[node].insert(matchId);
    else // Attempt to recurse...
    {
        for (xmlNodePtr child = xmlFirstElementChild(node);
            child != NULL; child = child->next)
          markLexicals(child, matchDepth, matchId);
    }

}

std::vector<LexItem> *LexItem::collectLexicals(xmlDoc *doc,
    QHash<xmlNode *, QSet<size_t> > const &matchDepth)
{
    std::vector<LexItem> *items = new std::vector<LexItem>;

    xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
    if (xpCtx == 0)
    {
        qDebug() << "Could not make XPath context.";
        return items;
    }

    xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
        toXmlStr("//node[@word]"), xpCtx);
    if (xpObj == 0) {
        qDebug() << "Could not make XPath expression to select active nodes.";
        xmlXPathFreeContext(xpCtx);
        return items;
    }

    xmlNodeSet *nodeSet = xpObj->nodesetval;
    if (nodeSet != 0)
    {
        for (int i = 0; i < nodeSet->nodeNr; ++i)
        {
            xmlNode *node = nodeSet->nodeTab[i];

            if (node->type == XML_ELEMENT_NODE)
            {
                xmlAttrPtr wordAttr = xmlHasProp(node, toXmlStr("word"));
                xmlChar *word = xmlNodeGetContent(wordAttr->children);

                xmlAttrPtr beginAttr = xmlHasProp(node, toXmlStr("begin"));
                size_t begin = 0;
                if (beginAttr)
                {
                    xmlChar *beginStr = xmlNodeGetContent(beginAttr->children);
                    try {
                        begin = parseString<size_t>(fromXmlStr(beginStr));
                    } catch (std::invalid_argument &e) {
                        qDebug() << e.what();
                    }
                }

                LexItem item = {fromXmlStr(word), begin, matchDepth[node] };

                items->push_back(item);
            }
        }
    }

    std::sort(items->begin(), items->end());

    return items;
}

std::vector<LexItem> *LexItem::parseSentence(QString const &treeStr)
{
    QByteArray xmlData(treeStr.toUtf8());

    xmlDocPtr doc;
    doc = xmlReadMemory(xmlData.constData(), xmlData.size(), NULL, NULL, 0);
    if (doc == NULL)
        return new std::vector<LexItem>;

    // We get the sentence node, we should process its children.
    xmlNode *sentenceNode = xmlDocGetRootElement(doc);
    if (sentenceNode == NULL) {
        xmlFreeDoc(doc);
        return new std::vector<LexItem>;
    }

    xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
    if (xpCtx == 0)
    {
        qDebug() << "Could not make XPath context.";
        xmlFreeDoc(doc);
        return new std::vector<LexItem>;
    }

    xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
        toXmlStr("//node[@active='1']"), xpCtx);
    if (xpObj == 0) {
        qDebug() << "Could not make XPath expression to select active nodes.";
        xmlXPathFreeContext(xpCtx);
        xmlFreeDoc(doc);
        return new std::vector<LexItem>;
    }

    // Do we have matches?
    xmlNodeSet *nodeSet = xpObj->nodesetval;
    QHash<xmlNode *, QSet<size_t> > matchDepth;
    if (nodeSet != 0)
    {
        for (int i = 0; i < nodeSet->nodeNr; ++i)
          if (nodeSet->nodeTab[i]->type == XML_ELEMENT_NODE)
              markLexicals(nodeSet->nodeTab[i], &matchDepth, i);
    }

    std::vector<LexItem> *items = collectLexicals(doc, matchDepth);

    xmlXPathFreeObject(xpObj);
    xmlXPathFreeContext(xpCtx);
    xmlFreeDoc(doc);

    return items;
}
