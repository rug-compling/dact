#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

//for debugging only
#include <iostream>

#include <libxml/tree.h>

#include "SimpleDTD.hh"

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
    AttributeMap &attributeMap = *reinterpret_cast<AttributeMap *>(data);

    char const *attributeName(reinterpret_cast<char const *>(attr->name));
    AttributeMap::iterator attributeMapPos = attributeMap.find(attributeName);
    
    switch (attr->atype)
    {
        case XML_ATTRIBUTE_CDATA:
        {
            if (attributeMapPos == attributeMap.end())
                attributeMap[attributeName] = new SimpleDTDCDATAAttribute();
            else
                std::cerr << "Warning: are we redefining " << attributeName << "?" << std::endl;

            break;
        }

        case XML_ATTRIBUTE_NMTOKEN:
        {
            if (attributeMapPos == attributeMap.end())
                attributeMap[attributeName] = new SimpleDTDNameTokenAttribute();
            else
                std::cerr << "Warning: are we redefining " << attributeName << "?" << std::endl;

            break;
        }

        case XML_ATTRIBUTE_ENUMERATION:
        {
            SimpleDTDEnumerationAttribute *simpleAttr = 0;

            if (attributeMapPos != attributeMap.end())
                simpleAttr = reinterpret_cast<SimpleDTDEnumerationAttribute*>(attributeMapPos->second);
            else
            {
                simpleAttr = new SimpleDTDEnumerationAttribute();
                attributeMap[attributeName] = simpleAttr;
            }

            for (xmlEnumerationPtr value = attr->tree; value != 0; value = value->next)
                simpleAttr->addValue(reinterpret_cast<char const *>(value->name));

            break;
        }

        default:
            std::cout << "Attribute of type " << attr->atype << std::endl;
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

    xmlDtdPtr dtd = xmlIOParseDTD(NULL, input, XML_CHAR_ENCODING_8859_1);

    if (dtd == 0)
        throw std::runtime_error("Could not parse DTD");

    if (dtd->elements == 0)
        throw std::runtime_error("DTD hashtable has no elements");

    xmlHashScan(reinterpret_cast<xmlHashTablePtr>(dtd->elements), scanElement, &d_elements);

    xmlHashScan(reinterpret_cast<xmlHashTablePtr>(dtd->attributes), scanAttribute, &d_attributes);

    xmlFreeDtd(dtd);
}

SimpleDTD::~SimpleDTD()
{
    // Free the SimpleDTDAttribute objects in the attribute map.
    for (AttributeMap::iterator it = d_attributes.begin(); it != d_attributes.end(); ++it)
        delete it->second;
}

bool SimpleDTD::allowElement(std::string const &element, std::string const &parent) const
{
    return d_elements.find(element) != d_elements.end();
}

bool SimpleDTD::allowAttribute(std::string const &attribute, std::string const &element) const
{
    if (element == "*")
        return d_attributes.find(attribute) != d_attributes.end();

    ElementMap::const_iterator pos = d_elements.find(element);

    if (pos == d_elements.end())
        return false;

    return pos->second.count(attribute) > 0;
}

bool SimpleDTD::allowValueForAttribute(std::string const &value, std::string const &attribute) const
{
    AttributeMap::const_iterator pos = d_attributes.find(attribute);

    if (pos == d_attributes.end())
        return false;

    return pos->second->test(value);
}

ElementMap const &SimpleDTD::elementMap()
{
    return d_elements;
}