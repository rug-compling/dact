#include <libxml/xpath.h>

#include "XPathValidator.hh"

void ignoreStructuredError(void *userdata, xmlErrorPtr err)
{
}

XPathValidator::XPathValidator(QObject *parent, bool variables) :
    QValidator(parent),
    d_variables(variables)
{
}

XPathValidator::XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent, bool variables) :
    QValidator(parent),
    d_variables(variables),
    d_macrosModel(macrosModel)
{
}


XPathValidator::State XPathValidator::validate(QString &exprStr, int &pos) const
{
    if (d_corpusReader.isNull())
        return XPathValidator::Intermediate;
    
    if (exprStr.trimmed().isEmpty())
        return XPathValidator::Acceptable;

    // Consistent quoting
    exprStr.replace('\'', '"');

    
    QString expandedExpr = d_macrosModel.isNull()
        ? exprStr
        : d_macrosModel->expand(exprStr); 
    
    bool valid = d_corpusReader->isValidQuery(alpinocorpus::CorpusReader::XPATH, d_variables,
        expandedExpr);

    return valid ? XPathValidator::Acceptable : XPathValidator::Intermediate;
}
