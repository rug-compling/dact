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
    QString filename(index.sibling(index.row(), 0).data(Qt::UserRole).toString());
    if (!d_cache.contains(filename))
    {
        ac::CorpusReader::MarkerQuery query(
            reinterpret_cast<FilterModel const *>(
                index.model())->lastQuery().toUtf8().constData(),
            "active", "1");
        std::list<ac::CorpusReader::MarkerQuery> queries;
        queries.push_back(query);

        QString xmlData = QString::fromUtf8(d_corpus->read(
            filename.toUtf8().constData(), queries).c_str());

        std::list<Chunk> *chunks = Chunk::parseSentence(xmlData);

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
