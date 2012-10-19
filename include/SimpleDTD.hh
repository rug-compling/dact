#ifndef SIMPLE_DTD_HH
#define SIMPLE_DTD_HH

#include <string>
#include <map>
#include "SimpleDTDAttribute.hh"

typedef std::map<std::string, std::set<std::string> > ElementMap;

typedef std::map<std::string, SimpleDTDAttribute*> AttributeMap;

class SimpleDTD
{
public:
	SimpleDTD(std::string const &data);
	~SimpleDTD();

	bool allowElement(std::string const &element, std::string const &parent) const;
	bool allowAttribute(std::string const &attribute, std::string const &element) const;
	bool allowValueForAttribute(std::string const &value, std::string const &attribute) const;
private:
    ElementMap d_elements;
    AttributeMap d_attributes;
};

#endif // SIMPLE_DTD_HH
