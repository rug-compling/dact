#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

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
}

SimpleDTD::SimpleDTD(std::string const &data)
{
    xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(data.c_str(),
        data.size(), XML_CHAR_ENCODING_8859_1);

    xmlDtdPtr dtd = xmlIOParseDTD(NULL, input, XML_CHAR_ENCODING_8859_1);

    if (dtd == 0)
        throw std::runtime_error("Could not parse DTD");

    if (dtd->elements == 0)
        throw std::runtime_error("DTD hashtable has no elements");

    xmlHashScan(reinterpret_cast<xmlHashTablePtr>(dtd->elements), scanElement, &d_map);

    for (ElementMap::const_iterator elemIter = d_map.begin();
        elemIter != d_map.end(); ++elemIter)
    std::copy(elemIter->second.begin(), elemIter->second.end(),
        inserter(d_attributes, d_attributes.begin()));
}

bool SimpleDTD::allowElement(std::string const &element, std::string const &parent) const
{
    return d_map.find(element) != d_map.end();
}

bool SimpleDTD::allowAttribute(std::string const &attribute, std::string const &element) const
{
    if (element == "*")
        return d_attributes.find(attribute) != d_attributes.end();

    ElementMap::const_iterator pos = d_map.find(element);

    if (pos == d_map.end())
        return false;

    return pos->second.count(attribute) > 0;
}