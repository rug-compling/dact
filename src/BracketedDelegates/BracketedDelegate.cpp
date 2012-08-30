#include <iostream>
#include <list>

#include "BracketedDelegate.hh"
#include "Chunk.hh"
#include "FilterModel.hh"

namespace ac = alpinocorpus;

BracketedDelegate::BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent)
:
    QStyledItemDelegate(parent),
    d_corpus(corpus)
{}

std::list<Chunk> BracketedDelegate::parseChunks(QModelIndex const &index) const
{
    QString filename(index.data(Qt::UserRole).toString());

    if (!d_cache.contains(filename))
    {
        QString bracketed_sentence(
            index.sibling(index.row(), 2).data(Qt::UserRole).toString().trimmed());

        std::list<Chunk> *chunks = Chunk::parseSentence(bracketed_sentence);

        d_cache.insert(filename, chunks);
    }

    return *d_cache[filename];
}

QString BracketedDelegate::bracketedSentence(QModelIndex const &index) const
{
    QString sent;
    QTextStream sentStream(&sent);

    std::list<Chunk> chunks = parseChunks(index);

    size_t prevDepth = 0;
    foreach (Chunk const &chunk, chunks)
    {
        if (chunk.text().isEmpty())
          continue;

        if (chunk.depth() != prevDepth) {
          if (prevDepth < chunk.depth())
            sentStream << QString(chunk.depth() - prevDepth, QChar('['));
          else
            sentStream << QString(prevDepth - chunk.depth(), QChar(']'));

          prevDepth = chunk.depth();
        }

        sentStream << chunk.text();
    }

    return sent.trimmed();
}

QString BracketedDelegate::sentenceForClipboard(QModelIndex const &index) const
{
    return bracketedSentence(index);
}
