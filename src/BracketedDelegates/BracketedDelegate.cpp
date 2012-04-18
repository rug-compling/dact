#include <list>

#include "BracketedDelegate.hh"
#include "FilterModel.hh"

namespace ac = alpinocorpus;

BracketedDelegate::BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent)
:
    QStyledItemDelegate(parent),
    d_corpus(corpus)
{}

QList<BracketedDelegate::Chunk> BracketedDelegate::parseChunks(QModelIndex const &index) const
{
    QString filename(index.data(Qt::UserRole).toString());

    if (!d_cache.contains(filename))
    {
        QString bracketed_sentence(bracketedSentence(index));

        QList<Chunk> *chunks = parseSentence(bracketed_sentence);

        d_cache.insert(filename, chunks);
    }

    return *d_cache[filename];
}

QList<BracketedDelegate::Chunk> *BracketedDelegate::parseSentence(QString const &sentence) const
{
    QList<Chunk> *chunks = new QList<Chunk>;
    QRegExp brackets("\\[|\\]");

    int depth = 0, pos = -1, readTill = 0;

    while ((pos = sentence.indexOf(brackets, readTill)) != -1)
    {
        // reading one char less on the left and right to omit the bracktes surrounding the match.
        // @TODO use xml for this instead of silly brackets. Maybe even merge this parser with the
        // one in DactTreeScene and keep theses parsed trees in memory to speed things up.

        // @TODO this code is quite a mess. This could be a lot more elegant. I hope…

        // search backwards for the opening bracket of this match by stepping over the submatches
        int openingBracketPos = readTill;
        int subMatches = 0;
        while (openingBracketPos > 0)
        {
            openingBracketPos = sentence.lastIndexOf(brackets, openingBracketPos - 1);

            if (openingBracketPos == -1)
            {
                openingBracketPos = 0;
                break;
            }

            if (sentence[openingBracketPos] == ']')
            {
                ++subMatches;
            }
            else if(sentence[openingBracketPos] == '[')
            {
                if (!subMatches)
                    break;
                else
                    --subMatches;
            }
        }

        // find the closing bracket for this level of the match
        int closingBracketPos = pos - 1; // -1 because then we first look at pos. If pos is ], no need to look further.
        subMatches = 0;
        while (closingBracketPos < sentence.length())
        {
            closingBracketPos = sentence.indexOf(brackets, closingBracketPos + 1);

            if (closingBracketPos == -1)
            {
                closingBracketPos = pos;
                break;
            }

            if (sentence[closingBracketPos] == '[')
            {
                ++subMatches;
            }
            else if(sentence[closingBracketPos] == ']')
            {
                if (!subMatches)
                    break;
                else
                    --subMatches;
            }
        }


        chunks->append(Chunk(depth,
            sentence.left(readTill == 0 ? readTill : readTill - 1),
            sentence.mid(readTill, pos - readTill),
            sentence.mid(openingBracketPos + 1, closingBracketPos - openingBracketPos - 1), // -1 to omit the closing bracket
            sentence.mid(pos + 1),
            sentence.mid(closingBracketPos + 1))); // +1 to omit the closing bracket

        readTill = pos + 1;

        if (sentence[pos] == '[')
            ++depth;

        else if (sentence[pos] == ']')
            --depth;
    }

    chunks->append(Chunk(depth, sentence.left(readTill), sentence.mid(readTill), "", "", ""));

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

BracketedDelegate::Chunk::Chunk(int depth, QString const &left, QString const &text, QString const &fullText, QString const &right, QString const &remainingRight)
:
    d_depth(depth),
    d_left(left),
    d_text(text),
    d_fullText(fullText),
    d_right(right),
    d_remainingRight(remainingRight)
{}

int BracketedDelegate::Chunk::depth() const
{
    return d_depth;
}

QString const &BracketedDelegate::Chunk::left() const
{
    return d_left;
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
