#include <QtDebug>

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
   
    int openingBracketPos = base.indexOf('[', subSelectionPos);
    
    if (openingBracketPos == -1)
        return QString("%1[%2]").arg(base).arg(condition);

    // Add parenthesis around original predicates.
    QString expanded = base;
    expanded.insert(openingBracketPos + 1, '(');

    int closingBracketPos = expanded.lastIndexOf(']');
    if (closingBracketPos == -1) {
      qDebug() << "Malformed query?: " << base;
      return base;
    }

    expanded.insert(closingBracketPos, QString(") and %1").arg(condition));
    
    return expanded;
}
