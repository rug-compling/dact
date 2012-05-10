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
