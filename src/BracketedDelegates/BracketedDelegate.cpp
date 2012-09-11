#include <iostream>
#include <list>
#include <vector>

#include <QSet>

#include "BracketedDelegate.hh"
#include "LexItem.hh"
#include "FilterModel.hh"

namespace ac = alpinocorpus;

BracketedDelegate::BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent)
:
    QStyledItemDelegate(parent),
    d_corpus(corpus)
{}

std::vector<LexItem> const &BracketedDelegate::retrieveSentence(QModelIndex const &index) const
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

        std::vector<LexItem> *items = LexItem::parseSentence(xmlData);

        d_cache.insert(filename, items);
    }

    return *d_cache[filename];
}

QString BracketedDelegate::bracketedSentence(QModelIndex const &index) const
{
    QString sent;
    QTextStream sentStream(&sent);

    std::vector<LexItem> lexItems = retrieveSentence(index);

    size_t prevDepth = 0;
    foreach (LexItem const &lexItem, lexItems)
    {
        size_t depth = lexItem.matches.size();

        if (depth != prevDepth) {
          if (prevDepth < depth)
            sentStream << QString(depth - prevDepth, QChar('['));
          else
            sentStream << QString(prevDepth - depth, QChar(']'));

          prevDepth = depth;
        }

        sentStream << lexItem.word;
    }

    return sent.trimmed();
}

QString BracketedDelegate::sentenceForClipboard(QModelIndex const &index) const
{
    return bracketedSentence(index);
}
