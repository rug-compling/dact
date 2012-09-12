#include <iostream>
#include <list>
#include <vector>

#include <QStringList>
#include <QSet>

#include <AlpinoCorpus/LexItem.hh>

#include "BracketedDelegate.hh"
#include "FilterModel.hh"

namespace ac = alpinocorpus;

BracketedDelegate::BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent)
:
    QStyledItemDelegate(parent),
    d_corpus(corpus)
{}

std::vector<ac::LexItem> const &BracketedDelegate::retrieveSentence(QModelIndex const &index) const
{
    QString filename(index.sibling(index.row(), 0).data(Qt::UserRole).toString());
    if (!d_cache.contains(filename))
    {
        std::vector<ac::LexItem> *items = new std::vector<ac::LexItem>(
            d_corpus->sentence(filename.toUtf8().constData(),
            reinterpret_cast<FilterModel const *>(
                index.model())->lastQuery().toUtf8().constData()));

        d_cache.insert(filename, items);
    }

    return *d_cache[filename];
}

QString BracketedDelegate::bracketedSentence(QModelIndex const &index) const
{
    QStringList sent;

    std::vector<ac::LexItem> lexItems = retrieveSentence(index);

    size_t prevDepth = 0;
    foreach (ac::LexItem const &lexItem, lexItems)
    {
        size_t depth = lexItem.matches.size();

        if (depth != prevDepth) {
          if (prevDepth < depth)
            sent.append(QString(depth - prevDepth, QChar('[')));
          else
            sent.append(QString(prevDepth - depth, QChar(']')));

          prevDepth = depth;
        }

        sent.append(QString::fromUtf8(lexItem.word.c_str()));
    }

    return sent.join(" ");
}

QString BracketedDelegate::sentenceForClipboard(QModelIndex const &index) const
{
    return bracketedSentence(index);
}
