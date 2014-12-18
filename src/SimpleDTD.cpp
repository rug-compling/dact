#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include <QScopedPointer>
#include <QtDebug>

#include <libxml/tree.h>

#include <SimpleDTD.hh>
#include <XMLDeleters.hh>

namespace {

void scanElement(void *payload, void *data, xmlChar *name)
{
    xmlElement *elem = reinterpret_cast<xmlElement*>(payload);
    ElementMap *elements = reinterpret_cast<ElementMap *>(data);

    std::string elemName = reinterpret_cast<char const *>(elem->name);

    // Some elements do not have attributes (e.g. 'sentence' in alpino_ds).
    if (elements->find(elemName) == elements->end())
        (*elements)[elemName] = std::set<std::string>();


    for (xmlAttributePtr attr = elem->attributes; attr != NULL; attr = reinterpret_cast<xmlAttributePtr>(attr->next))
        (*elements)[elemName].insert(reinterpret_cast<char const *>(attr->name));
}

void scanAttribute(void *payload, void *data, xmlChar *name)
{
    xmlAttribute *attr = reinterpret_cast<xmlAttribute*>(payload);
    ElementAttributeMap &attributeMap = *reinterpret_cast<ElementAttributeMap *>(data);

    char const *elementName(reinterpret_cast<char const *>(attr->elem));
    char const *attributeName(reinterpret_cast<char const *>(attr->name));

    AttributeMap::iterator attributeMapPos = attributeMap[elementName].find(attributeName);
    ElementAttributeMap::iterator elementIter = attributeMap.find(elementName);
    
    switch (attr->atype)
    {
        case XML_ATTRIBUTE_CDATA:
        {
            if (attributeMapPos == elementIter->second.end())
                elementIter->second[attributeName] =
                    QSharedPointer<SimpleDTDAttribute>(new SimpleDTDCDATAAttribute());
            else
                qWarning() << "Warning: are we redefining " << attributeName << "?";

            break;
        }

        case XML_ATTRIBUTE_NMTOKEN:
        {
            if (attributeMapPos == elementIter->second.end())
                elementIter->second[attributeName] =
                    QSharedPointer<SimpleDTDAttribute>(new SimpleDTDNameTokenAttribute());
            else
                qWarning() << "Warning: are we redefining " << attributeName << "?";

            break;
        }

        case XML_ATTRIBUTE_ENUMERATION:
        {
            QSharedPointer<SimpleDTDEnumerationAttribute> simpleAttr;

            if (attributeMapPos != elementIter->second.end()) {
                simpleAttr = qSharedPointerDynamicCast<SimpleDTDEnumerationAttribute>(
                    attributeMapPos->second);

                if (simpleAttr.isNull()) {
                    qWarning() << "Attribute '" << attributeName <<
                        "' used twice and incompatibly, skipping!";
                    break;
                }
            }
            else
            {
                simpleAttr = QSharedPointer<SimpleDTDEnumerationAttribute>(
                    new SimpleDTDEnumerationAttribute());
                elementIter->second[attributeName] = simpleAttr;
            }

            for (xmlEnumerationPtr value = attr->tree; value != 0; value = value->next)
                simpleAttr->addValue(reinterpret_cast<char const *>(value->name));

            break;
        }

        default:
            qWarning() << "Attribute of type " << attr->atype;
            break;
    }
}

}

SimpleDTD::SimpleDTD(std::string const &data)
{
    xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(data.c_str(),
        data.size(), XML_CHAR_ENCODING_8859_1);

    if (input == 0)
        throw std::runtime_error("Could not read DTD input.");

    QScopedPointer<xmlDtd, XmlDtdDeleter> dtd(
        xmlIOParseDTD(NULL, input, XML_CHAR_ENCODING_UTF8));

    if (dtd == 0)
        throw std::runtime_error("Could not parse DTD");

    if (dtd->elements == 0)
        throw std::runtime_error("DTD hashtable has no elements");

    xmlHashScan(reinterpret_cast<xmlHashTablePtr>(dtd->elements), scanElement, &d_elements);

    xmlHashScan(reinterpret_cast<xmlHashTablePtr>(dtd->attributes), scanAttribute, &d_attributes);
}

SimpleDTD::~SimpleDTD()
{
}

bool SimpleDTD::allowElement(std::string const &element, std::string const &parent) const
{
    return d_elements.find(element) != d_elements.end();
}

bool SimpleDTD::allowAttribute(std::string const &attribute, std::string const &element) const
{
    if (element == "*") {
        // Loop over all elements, check corresponding attributes.
        for (ElementAttributeMap::const_iterator iter = d_attributes.begin();
            iter != d_attributes.end(); ++iter)
        {
            if (iter->second.find(attribute) != iter->second.end())
                return true;
        }

        return false;
    }

    ElementMap::const_iterator pos = d_elements.find(element);

    if (pos == d_elements.end())
        return false;

    return pos->second.count(attribute) > 0;
}

bool SimpleDTD::allowValueForAttribute(std::string const &value, std::string const &attribute,
    std::string const &element) const
{
    ElementAttributeMap::const_iterator iter = d_attributes.find(element);
    if (iter == d_attributes.end())
        return false;

    // We have found the element, see if it has the attribute.
    AttributeMap::const_iterator attrIter = iter->second.find(attribute);

    if (attrIter == iter->second.end())
        return false;

    return attrIter->second->test(value);
}

ElementMap const &SimpleDTD::elementMap()
{
    return d_elements;
}
