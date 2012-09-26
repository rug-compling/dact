#include <QSharedPointer>

#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathValidator.hh"

namespace ac = alpinocorpus;

void ignoreStructuredError(void *userdata, xmlErrorPtr err)
{
}

XPathValidator::XPathValidator(QObject *parent, bool variables,
    QSharedPointer<ac::CorpusReader> corpusReader) :
    QValidator(parent),
    d_variables(variables),
    d_corpusReader(corpusReader)
{
}

XPathValidator::XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent, bool variables,
    QSharedPointer<ac::CorpusReader> corpusReader) :
    QValidator(parent),
    d_variables(variables),
    d_macrosModel(macrosModel),
    d_corpusReader(corpusReader)
{
}


XPathValidator::State XPathValidator::validate(QString &exprStr, int &pos) const
{
    if (d_corpusReader.isNull())
        return XPathValidator::Intermediate;
    
    if (exprStr.trimmed().isEmpty())
        return XPathValidator::Acceptable;
    
    QString expandedExpr = d_macrosModel.isNull()
        ? exprStr
        : d_macrosModel->expand(exprStr); 
    
    bool valid = d_corpusReader->isValidQuery(alpinocorpus::CorpusReader::XPATH, d_variables,
        expandedExpr.toUtf8().constData()).isRight();

    return valid ? XPathValidator::Acceptable : XPathValidator::Intermediate;
}
