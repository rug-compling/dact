#include "Query.hh"

QString generateQuery(QString const &base, QString const &attribute, QString const &value)
{
    QString condition = QString("@%1=\"%2\"").arg(attribute).arg(value);
    
    return generateQuery(base, condition);
}

QString generateQuery(QString const &base, QString const &condition)
{
    int subSelectionPos = base.lastIndexOf('/');
    
    if (!subSelectionPos)
        return QString();
    
    int closingBracketPos = base.mid(subSelectionPos).lastIndexOf(']');
    
    if (closingBracketPos == -1)
        return QString("%1[%2]").arg(base).arg(condition);
    else
        return QString("%1 and %2%3").arg(
            base.left(subSelectionPos + closingBracketPos),
            condition,
            base.mid(subSelectionPos + closingBracketPos));
}
