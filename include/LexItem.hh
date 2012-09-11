#ifndef LEX_ITEM_HH
#define LEX_ITEM_HH

#include <list>
#include <vector>

#include <QHash>
#include <QSet>
#include <QString>

struct _xmlDoc;
struct _xmlNode;
typedef struct _xmlDoc xmlDoc;
typedef struct _xmlNode xmlNode;

struct LexItem
{
    QString word;
    size_t begin;
    QSet<size_t> matches;

    inline bool operator<(LexItem const &other) const
    {
        if (begin != other.begin)
          return begin < other.begin;
        else
          return word < other.word;
    }

    static std::vector<LexItem> *parseSentence(QString const &sentence);

private:
    static void markLexicals(xmlNode *node,
        QHash<xmlNode *, QSet<size_t> > *matchDepth, size_t matchId);
    static std::vector<LexItem> *collectLexicals(xmlDoc *node,
        QHash<xmlNode *, QSet<size_t> > const &matchDepth);
};

#endif // LEX_ITEM_HH
