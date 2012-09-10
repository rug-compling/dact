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

#include "Chunk.hh"
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
void Chunk::markLexicals(xmlNode *node, QHash<xmlNode *, size_t> *matchDepth)
{
    // Don't attempt to handle a node that we can't.
    if (node->type != XML_ELEMENT_NODE ||
          std::strcmp(reinterpret_cast<char const *>(node->name), "node") != 0)
        return;

    xmlAttrPtr wordProp = xmlHasProp(node, toXmlStr("word"));
    if (wordProp != 0)
        ++(*matchDepth)[node];
    else // Attempt to recurse...
    {
        for (xmlNodePtr child = xmlFirstElementChild(node);
            child != NULL; child = child->next)
          markLexicals(child, matchDepth);
    }

}

std::vector<Chunk::LexItem> Chunk::collectLexicals(xmlDoc *doc,
    QHash<xmlNode *, size_t> const &matchDepth)
{
    std::vector<LexItem> items;

    xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
    if (xpCtx == 0)
    {
        qDebug() << "Could not make XPath context.";
        return items;
    }

    xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>("//node[@word]"), xpCtx);
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

                items.push_back(item);
            }
        }
    }

    std::sort(items.begin(), items.end());

    return items;
}


std::list<Chunk> *Chunk::createChunks(std::vector<LexItem> const &lexicals)
{
    std::list<Chunk> *chunks = new std::list<Chunk>;

    for (std::vector<LexItem>::const_iterator iter = lexicals.begin();
        iter != lexicals.end(); ++iter)
    {
        // Append if the current lex item has the same depth as the
        // previous.
        if (chunks->size() != 0 &&
                iter->matchDepth == chunks->back().depth())
            chunks->back().setText(chunks->back().text() + " " + iter->word);
        else
        {
            if (chunks->size() != 0)
                chunks->back().setText(QString("%1 ").arg(chunks->back().text()));

            chunks->push_back(Chunk(iter->matchDepth, QString(), iter->word,
                QString(), QString(), QString()));
        }
    }

    // Fill left context.
    QString left;
    for (std::list<Chunk>::iterator iter = chunks->begin();
            iter != chunks->end(); ++iter) {
        iter->setLeft(left);
        left += iter->text();
    }

    // Fill right context.
    QString right;
    for (std::list<Chunk>::reverse_iterator iter = chunks->rbegin();
            iter != chunks->rend(); ++iter) {
        iter->setRight(right);
        right.prepend(iter->text());
    }

    for (std::list<Chunk>::iterator iter = chunks->begin();
        iter != chunks->end(); ++iter)
    {
        QStringList rightList;
        std::list<Chunk>::iterator iterRight = iter;
        ++iterRight; // Skip ourselves.
        while (iterRight != chunks->end())
        {
            if (iterRight->depth() < iter->depth())
                break;

            rightList.push_back(iterRight->text());

            ++iterRight;
        }

        QStringList remainingRightList;
        while (iterRight != chunks->end())
        {
          remainingRightList.push_back(iterRight->text());
          ++iterRight;
        }

        iter->setFullText(QString("%1%2").arg(iter->text()).arg(rightList.join("")));
        iter->setRemainingRight(remainingRightList.join(""));
    }

    return chunks;
}

void Chunk::processTree(xmlNode *node, size_t depth, std::list<Chunk> *chunks) {
    for (; node; node = node->next) {
        // Element node, recurse, increase depth.
        if (node->type == XML_ELEMENT_NODE &&
                std::strcmp(reinterpret_cast<char const *>(node->name), "bracket") == 0)
            processTree(node->children, depth + 1, chunks);
        else if (node->type == XML_TEXT_NODE) {
            xmlChar *cText = xmlNodeGetContent(node->parent);
            QString text;
            if (cText != NULL) {
                text = QString::fromUtf8(reinterpret_cast<const char *>(cText));
                xmlFree(cText);
            }

            // Get text right of this bracket (including subbrackets): traverse
            // up the tree, getting the right context of every node.
            QStringList rightList;
            xmlNode *ancestor = node->parent;
            while (ancestor != NULL) {
                for (xmlNode *right = ancestor->next; right; right = right->next)
                {
                    cText = xmlNodeGetContent(right);
                    if (cText != NULL) {
                        rightList.append(QString::fromUtf8(reinterpret_cast<const char *>(cText)));
                        xmlFree(cText);
                    }
                }
                ancestor = ancestor->parent;
            }

            chunks->push_back(Chunk(depth, QString(),
                QString::fromUtf8(reinterpret_cast<const char *>(node->content)),
                text, QString(), rightList.join(" ")));
        }
    }
}

std::list<Chunk> *Chunk::parseSentence(QString treeStr)
{
    QByteArray xmlData(treeStr.toUtf8());

    xmlDocPtr doc;
    doc = xmlReadMemory(xmlData.constData(), xmlData.size(), NULL, NULL, 0);
    if (doc == NULL)
        return new std::list<Chunk>;

    // We get the sentence node, we should process its children.
    xmlNode *sentenceNode = xmlDocGetRootElement(doc);
    if (sentenceNode == NULL) {
        xmlFreeDoc(doc);
        return new std::list<Chunk>;
    }

    xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
    if (xpCtx == 0)
    {
        qDebug() << "Could not make XPath context.";
        xmlFreeDoc(doc);
        return new std::list<Chunk>;
    }

    xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>("//node[@active='1']"), xpCtx);
    if (xpObj == 0) {
        qDebug() << "Could not make XPath expression to select active nodes.";
        xmlXPathFreeContext(xpCtx);
        xmlFreeDoc(doc);
        return new std::list<Chunk>;
    }

    // Do we have matches?
    xmlNodeSet *nodeSet = xpObj->nodesetval;
    QHash<xmlNode *, size_t> matchDepth;
    if (nodeSet != 0)
    {
        for (int i = 0; i < nodeSet->nodeNr; ++i)
          if (nodeSet->nodeTab[i]->type == XML_ELEMENT_NODE)
              markLexicals(nodeSet->nodeTab[i], &matchDepth);
    }

    std::vector<LexItem> items = collectLexicals(doc, matchDepth);

    std::list<Chunk> *chunks = createChunks(items);


    xmlXPathFreeObject(xpObj);
    xmlXPathFreeContext(xpCtx);
    xmlFreeDoc(doc);

    return chunks;
}

Chunk::Chunk(size_t depth, QString const &left, QString const &text, QString const &fullText, QString const &right, QString const &remainingRight)
:
    d_depth(depth),
    d_left(left),
    d_text(text),
    d_fullText(fullText),
    d_right(right),
    d_remainingRight(remainingRight)
{}

size_t Chunk::depth() const
{
    return d_depth;
}

QString const &Chunk::left() const
{
    return d_left;
}

void Chunk::setLeft(QString const &left)
{
    d_left = left;
}

void Chunk::setRemainingRight(QString const &right)
{
    d_remainingRight = right;
}

void Chunk::setRight(QString const &right)
{
    d_right = right;
}


void Chunk::setText(QString const &text)
{
  d_text = text;
}

QString const &Chunk::text() const
{
    return d_text;
}

void Chunk::setFullText(QString const &text)
{
    d_fullText = text;
}

QString const &Chunk::fullText() const
{
    return d_fullText;
}

QString const &Chunk::right() const
{
    return d_right;
}

QString const &Chunk::remainingRight() const
{
    return d_remainingRight;
}

QString Chunk::sentence() const
{
    return d_left + d_text + d_right;
}
