#ifndef XPATHVALIDATOR_HH
#define XPATHVALIDATOR_HH

#include <QObject>
#include <QSharedPointer>
#include <QValidator>

#include <AlpinoCorpus/CorpusReader.hh>

#include "DactMacrosModel.hh"

namespace ac = alpinocorpus;

/*!
 This class is used by the QLineEdit widgets for xpath queries. It uses
 the DactMacrosModel to support queries with macros in them.
 */
class XPathValidator : public QValidator
{
    Q_OBJECT
public:
    XPathValidator(QObject *parent = 0, bool variables = false,
        QSharedPointer<ac::CorpusReader> corpusReader = QSharedPointer<ac::CorpusReader>());
    XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent = 0, bool variables = false,
        QSharedPointer<ac::CorpusReader> corpusReader = QSharedPointer<ac::CorpusReader>());
    void setCorpusReader(QSharedPointer<ac::CorpusReader> corpusReader);
    State validate(QString &exprStr, int &pos) const;
private:
    bool d_variables;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<ac::CorpusReader> d_corpusReader;
};

inline void XPathValidator::setCorpusReader(QSharedPointer<ac::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
}

#endif // XPATHVALIDATOR_HH
