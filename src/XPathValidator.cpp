#include <QByteArray>
#include <QFile>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtDebug>

#include <xqilla/xqilla-simple.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/XQFunctionCoercion.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQOperator.hpp>
#include <xqilla/ast/XQPartialApply.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/axis/NodeTest.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>


#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include <QueryScope.hh>
#include <SimpleDTD.hh>
#include <XPathValidator.hh>

namespace ac = alpinocorpus;

namespace {
static XQilla s_xqilla;

void ignoreStructuredError(void *userdata, xmlErrorPtr err)
{
}

std::string getLiteral(VectorOfASTNodes const &nodes)
{
    for (VectorOfASTNodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        if ((*it)->getType() == ASTNode::LITERAL)
        {
            XQLiteral *literal = dynamic_cast<XQLiteral*>(*it);
            char *lit = xercesc::XMLString::transcode(literal->getValue());
            std::string strLit(lit);
            xercesc::XMLString::release(&lit);
            return strLit;
        }

    return std::string();
}

std::string getAttribute(VectorOfASTNodes const &nodes)
{
    ASTNode *expression = 0;

    // First, find the Atomize node

    for (VectorOfASTNodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        if ((*it)->getType() == ASTNode::ATOMIZE)
        {
            XQAtomize *atomize = dynamic_cast<XQAtomize*>(*it);
            expression = atomize->getExpression();
            break;
        }
    
    if (expression == 0)
        return std::string();

    // Then, if there is a Navigation node, go to the last Step
    if (expression->getType() == ASTNode::NAVIGATION)
    {
        XQNav *nav = dynamic_cast<XQNav*>(expression);
        XQNav::Steps steps(nav->getSteps());

        expression = steps.back().step;
    }
    
    // Then, traverse the Step node and get the attribute name
    if (expression->getType() == ASTNode::STEP)
    {
        XQStep *step = dynamic_cast<XQStep*>(expression);
        NodeTest *test = step->getNodeTest();

        if (test->getNodeType() == 0)
            return std::string();

        char *nodeType = xercesc::XMLString::transcode(test->getNodeType());
        if (nodeType == 0)
            return std::string();

        if (strcmp(nodeType, "attribute") == 0) {
            xercesc::XMLString::release(&nodeType);
            return xercesc::XMLString::transcode(test->getNodeName());
        }
    }

    // we failed.
    return std::string();
}

bool inspect(ASTNode *node, QSharedPointer<QueryScope> scope, SimpleDTD const &dtd)
{
    switch (node->getType())
    {
        case ASTNode::NAVIGATION:
        {
            XQNav *nav = dynamic_cast<XQNav*>(node);
            XQNav::Steps steps(nav->getSteps());

            for (XQNav::Steps::const_iterator it = steps.begin(); it != steps.end(); ++it)
                if (!inspect(it->step, scope, dtd))
                    return false;

            break;
        }

        case ASTNode::LITERAL:
            break;

        case ASTNode::NUMERIC_LITERAL:
            break;

        case ASTNode::QNAME_LITERAL:
            break;

        case ASTNode::SEQUENCE:
            break;

        case ASTNode::FUNCTION:
        {
            XQFunction *fun = dynamic_cast<XQFunction *>(node);

            VectorOfASTNodes const &args(fun->getArguments());

            for (VectorOfASTNodes::const_iterator it = args.begin();
                    it != args.end(); ++it)
                if (!inspect(*it, scope, dtd))
                    return false;

            break;
        }

        case ASTNode::VARIABLE:
            break;

        case ASTNode::STEP:
        {
            XQStep *step = dynamic_cast<XQStep*>(node);
            NodeTest *test = step->getNodeTest();

            // Wild cards have no element name.
            if (test->getNameWildcard())
            {
                scope->setNodeName("*");
                return true;
            }

            if (test->getNodeType() == 0 || test->getNodeName() == 0)
                return false;

            char *nodeType = xercesc::XMLString::transcode(test->getNodeType());
            char *nodeName = xercesc::XMLString::transcode(test->getNodeName());

            if (strcmp(nodeType, "element") == 0)
            {
                // Test to see if this element is allowed here
                if (!dtd.allowElement(nodeName, scope->nodeName()))
                    return false;

                scope->setNodeName(nodeName);
            }
            
            else if (strcmp(nodeType, "attribute") == 0)
            {
                if (!dtd.allowAttribute(nodeName, scope->nodeName()))
                    return false;
            }

            xercesc::XMLString::release(&nodeType);
            xercesc::XMLString::release(&nodeName);

            break;
        }

        case ASTNode::IF:
            break;

        case ASTNode::CASTABLE_AS:
            break;

        case ASTNode::CAST_AS:
            break;

        case ASTNode::TREAT_AS:
            break;

        case ASTNode::OPERATOR:
        {
            XQOperator *op = dynamic_cast<XQOperator *>(node);
            char *operatorName = xercesc::XMLString::transcode(op->getOperatorName());
            VectorOfASTNodes const &args(op->getArguments());

            for (VectorOfASTNodes::const_iterator it = args.begin();
                    it != args.end(); ++it)
                if (!inspect(*it, scope, dtd))
                    return false;

            // If it is the comparison operator, test if the attribute can have the tested value
            if (strcmp(operatorName, "comp") == 0)
            {
                // Find the literal and the atomized operants
                std::string literal = getLiteral(args);
                std::string attribute = getAttribute(args); 
                
                if (!literal.empty() && !attribute.empty())
                    if (!dtd.allowValueForAttribute(literal, attribute, scope->nodeName()))
                        return false;
            }

            xercesc::XMLString::release(&operatorName);
            
            break;
        }

        case ASTNode::CONTEXT_ITEM:
            break;

        case ASTNode::DOM_CONSTRUCTOR:
            break;

        case ASTNode::QUANTIFIED:
            break;

        case ASTNode::TYPESWITCH:
            break;

        case ASTNode::VALIDATE:
            break;

        case ASTNode::FUNCTION_CALL:
            break;

        case ASTNode::USER_FUNCTION:
            break;

        case ASTNode::ORDERING_CHANGE:
            break;

        case ASTNode::FUNCTION_COERCION:
        {
            XQFunctionCoercion *coercion =
                reinterpret_cast<XQFunctionCoercion *>(node);
            return inspect(coercion->getExpression(), scope, dtd);
        }

        case ASTNode::PROMOTE_UNTYPED:
            break;

        case ASTNode::PROMOTE_NUMERIC:
            break;

        case ASTNode::PROMOTE_ANY_URI:
            break;

        case ASTNode::DOCUMENT_ORDER:
        {
            XQDocumentOrder *docOrder = dynamic_cast<XQDocumentOrder*>(node);
            if (!inspect(docOrder->getExpression(), scope, dtd))
                return false;

            break;
        }

        case ASTNode::PREDICATE:
        {
            XQPredicate *predicate = dynamic_cast<XQPredicate*>(node);
            QSharedPointer<QueryScope> stepScope(new QueryScope(*scope));
            if (!inspect(predicate->getExpression(), stepScope, dtd))
                return false;

            if (!inspect(predicate->getPredicate(), stepScope, dtd))
                return false;

            break;
        }

        case ASTNode::ATOMIZE:
        {
            XQAtomize *atomize = dynamic_cast<XQAtomize*>(node);
            if (!inspect(atomize->getExpression(), scope, dtd))
                return false;

            break;
        }

        case ASTNode::EBV:
            break;

        case ASTNode::FTCONTAINS:
            break;

        case ASTNode::UDELETE:
            break;

        case ASTNode::URENAME:
            break;

        case ASTNode::UREPLACE:
            break;

        case ASTNode::UREPLACE_VALUE_OF:
            break;

        case ASTNode::UTRANSFORM:
            break;

        case ASTNode::UINSERT_AS_FIRST:
            break;

        case ASTNode::UINSERT_AS_LAST:
            break;

        case ASTNode::UINSERT_INTO:
            break;

        case ASTNode::UINSERT_AFTER:
            break;

        case ASTNode::UINSERT_BEFORE:
            break;

        case ASTNode::UAPPLY_UPDATES:
            break;

        case ASTNode::NAME_EXPRESSION:
            break;

        case ASTNode::CONTENT_SEQUENCE:
            break;

        case ASTNode::DIRECT_NAME:
            break;

        case ASTNode::RETURN:
            break;

        case ASTNode::NAMESPACE_BINDING:
            break;

        case ASTNode::FUNCTION_CONVERSION:
            break;

        case ASTNode::SIMPLE_CONTENT:
            break;

        case ASTNode::ANALYZE_STRING:
            break;

        case ASTNode::CALL_TEMPLATE:
            break;

        case ASTNode::APPLY_TEMPLATES:
            break;

        case ASTNode::INLINE_FUNCTION:
            break;

        case ASTNode::FUNCTION_REF:
            break;

        case ASTNode::FUNCTION_DEREF:
            break;

        case ASTNode::PARTIAL_APPLY:
        {
            XQPartialApply *partial =
                reinterpret_cast<XQPartialApply *>(node);

            if (!inspect(partial->getExpression(), scope, dtd))
                return false;

            VectorOfASTNodes const *args(partial->getArguments());

            for (VectorOfASTNodes::const_iterator it = args->begin();
                it != args->end(); ++it)
                if (!inspect(*it, scope, dtd))
                    return false;
            break;
        }

        case ASTNode::COPY_OF:
            break;

        case ASTNode::COPY:
            break;

        case ASTNode::MAP:
            break;

        case ASTNode::DEBUG_HOOK:
            break;
    }

    return true;
}

}

XPathValidator::XPathValidator(QObject *parent, bool variables,
    QSharedPointer<ac::CorpusReader> corpusReader) :
    QValidator(parent),
    d_variables(variables),
    d_corpusReader(corpusReader)
{
    parseDTD();
}

XPathValidator::XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent, bool variables,
    QSharedPointer<ac::CorpusReader> corpusReader) :
    QValidator(parent),
    d_variables(variables),
    d_macrosModel(macrosModel),
    d_corpusReader(corpusReader)
{
    parseDTD();
}

void XPathValidator::parseDTD()
{
    QString dtdPath = (d_corpusReader && d_corpusReader->type() == "tueba_tree") ?
      ":/dtd/tueba_tree.dtd" : ":/dtd/alpino_ds.dtd"; // XXX - hardcode?
    QFile dtdFile(dtdPath);

    if (!dtdFile.open(QFile::ReadOnly)) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): Could not read DTD.";
        return;
    }
    QByteArray dtdData(dtdFile.readAll());

    d_dtd = QSharedPointer<SimpleDTD>(new SimpleDTD(dtdData.constData()));
}

bool XPathValidator::checkAgainstDTD(QString const &query) const
{

    // If we cannot parse the query and traverse its AST without an exception, something
    // must be wrong with the query...
    try {

        QStringList subQueries = query.split("+|+");

        // Create an emptry document and associate namespace resolvers with it.
        AutoDelete<xercesc::DOMDocument> document(
            xercesc::DOMImplementation::getImplementation()->createDocument());
        AutoDelete<xercesc::DOMXPathNSResolver> resolver(
            document->createNSResolver(document->getDocumentElement()));
        resolver->addNamespaceBinding(X("fn"),
            X("http://www.w3.org/2005/xpath-functions"));
        resolver->addNamespaceBinding(X("xs"),
            X("http://www.w3.org/2001/XMLSchema"));

        foreach(QString subQuery, subQueries)
        {
            DynamicContext *ctx(s_xqilla.createContext(XQilla::XPATH2));
            ctx->setXPath1CompatibilityMode(true);
            ctx->setNSResolver(resolver);

            AutoDelete<XQQuery> xqQuery(s_xqilla.parse(X(subQuery.toUtf8().constData()), ctx));

            ASTNode *root = xqQuery->getQueryBody();
            // std::cout << xqQuery->getQueryPlan() << std::endl;

            QSharedPointer<QueryScope> rootScope(new QueryScope());
            rootScope->setNodeName("[document root]");


            if (!inspect(root, rootScope, *d_dtd))
                return false;
        }

        return true;
    } catch (XQException &e) {
        return false;
    }
}


void XPathValidator::setCorpusReader(QSharedPointer<ac::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
    parseDTD();
}

bool XPathValidator::validateAgainstDTD(QString const &exprStr) const
{
    QString expandedExpr = d_macrosModel.isNull()
        ? exprStr
        : d_macrosModel->expand(exprStr); 
    
    return checkAgainstDTD(expandedExpr);
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
