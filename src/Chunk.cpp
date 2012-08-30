#include <cstring>
#include <list>
#include <QString>
#include <QStringList>

extern "C" {
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
}

#include "Chunk.hh"

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

std::list<Chunk> *Chunk::parseSentence(QString sentence)
{
    sentence.replace("\n", "");
    sentence = sentence.simplified();
    QByteArray xmlData(sentence.toUtf8());

    xmlDocPtr doc;
    doc = xmlReadMemory(xmlData.constData(), xmlData.size(), NULL, NULL, 0);
    if (doc == NULL) {}

    // We get the sentence node, we should process its children.
    xmlNode *sentenceNode = xmlDocGetRootElement(doc);
    if (sentenceNode == NULL)
        return new std::list<Chunk>;

    xmlNode *node = sentenceNode->children;

    std::list<Chunk> *chunks = new std::list<Chunk>;

    processTree(node, 0, chunks);

    xmlFreeDoc(doc);

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


QString const &Chunk::text() const
{
    return d_text;
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
