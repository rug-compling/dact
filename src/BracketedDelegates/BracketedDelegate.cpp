#include <cstring>
#include <iostream>
#include <list>

extern "C" {
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
}

#include "BracketedDelegate.hh"
#include "FilterModel.hh"

namespace ac = alpinocorpus;

BracketedDelegate::BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent)
:
    QStyledItemDelegate(parent),
    d_corpus(corpus)
{}

std::list<BracketedDelegate::Chunk> BracketedDelegate::parseChunks(QModelIndex const &index) const
{
    QString filename(index.data(Qt::UserRole).toString());

    if (!d_cache.contains(filename))
    {
        QString bracketed_sentence(bracketedSentence(index));

        std::list<Chunk> *chunks = parseSentence(bracketed_sentence);

        d_cache.insert(filename, chunks);
    }

    return *d_cache[filename];
}

void BracketedDelegate::processTree(xmlNode *node, size_t depth, std::list<Chunk> *chunks) const {
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


std::list<BracketedDelegate::Chunk> *BracketedDelegate::parseSentence(QString sentence) const
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

QString BracketedDelegate::sentence(QModelIndex const &index) const
{

    // XXX - The corpus reader could be a remote corpus reader. So the
    //       method commented out below will be very slow. As a temporal
    //       workaround, we just read the bracketed sentence, and remove
    //       brackets. This is not good enough yet, because the sentence
    //       may contain brackets itself. On the other hand, how would
    //       the bracket parser handle such cases?!?
    //
    //       Alternatives:
    //
    //       - Read the sentence from the corpus with a stylesheet.
    //       - Escape brackets in sentences.
    QString s = bracketedSentence(index);
    s.remove(QChar('['));
    s.remove(QChar(']'));

    return s;

  // This method will be used in the sizeHint() of deriving classes.
  // Performing a full XML parse and XSLT transformation to get the sentence
  // length is *very* wasteful. Instead, we do some regexp magic.

  /*
  QString filename(index.data(Qt::UserRole).toString());
  QString data(QString::fromUtf8(d_corpus->read(filename.toUtf8().constData()).c_str()));
  QRegExp sentExpr("<sentence>(.*)</sentence>"); // XXX - Precompile once.
  int pos = sentExpr.indexIn(data);

  if (pos > -1)
    return sentExpr.cap(1);
  else
    return QString();
  */
}

QString BracketedDelegate::bracketedSentence(QModelIndex const &index) const
{
    return index.sibling(index.row(), 2).data(Qt::UserRole).toString().trimmed();
}

QString BracketedDelegate::sentenceForClipboard(QModelIndex const &index) const
{
    return bracketedSentence(index);
}

BracketedDelegate::Chunk::Chunk(size_t depth, QString const &left, QString const &text, QString const &fullText, QString const &right, QString const &remainingRight)
:
    d_depth(depth),
    d_left(left),
    d_text(text),
    d_fullText(fullText),
    d_right(right),
    d_remainingRight(remainingRight)
{}

size_t BracketedDelegate::Chunk::depth() const
{
    return d_depth;
}

QString const &BracketedDelegate::Chunk::left() const
{
    return d_left;
}

void BracketedDelegate::Chunk::setLeft(QString const &left)
{
    d_left = left;
}

void BracketedDelegate::Chunk::setRemainingRight(QString const &right)
{
    d_remainingRight = right;
}

void BracketedDelegate::Chunk::setRight(QString const &right)
{
    d_right = right;
}


QString const &BracketedDelegate::Chunk::text() const
{
    return d_text;
}

QString const &BracketedDelegate::Chunk::fullText() const
{
    return d_fullText;
}

QString const &BracketedDelegate::Chunk::right() const
{
    return d_right;
}

QString const &BracketedDelegate::Chunk::remainingRight() const
{
    return d_remainingRight;
}

QString BracketedDelegate::Chunk::sentence() const
{
    return d_left + d_text + d_right;
}
