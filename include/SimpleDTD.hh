#ifndef SIMPLE_DTD_HH
#define SIMPLE_DTD_HH

#include <map>
#include <set>
#include <string>

typedef std::map<std::string, std::set<std::string> > ElementMap;

class SimpleDTD
{
public:
	SimpleDTD(std::string const &data);

	bool allowElement(std::string const &element, std::string const &parent) const;
	bool allowAttribute(std::string const &attribute, std::string const &element) const;
private:
    ElementMap d_map;
    std::set<std::string> d_attributes;
};

#endif // SIMPLE_DTD_HH
