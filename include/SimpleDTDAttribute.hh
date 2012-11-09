#ifndef SIMPLE_DTD_ATTRIBUTE_HH
#define SIMPLE_DTD_ATTRIBUTE_HH

#include <map>
#include <set>
#include <string>

class SimpleDTDAttribute
{
public:
	virtual bool test(std::string const &value) const = 0;
	virtual ~SimpleDTDAttribute();
};

class SimpleDTDEnumerationAttribute : public SimpleDTDAttribute
{
	std::set<std::string> d_values;

public:
	void addValue(std::string const &value);

	bool test(std::string const &value) const;
};

class SimpleDTDNameTokenAttribute : public SimpleDTDAttribute
{
public:
	bool test(std::string const &value) const;
};

class SimpleDTDCDATAAttribute : public SimpleDTDAttribute
{
public:
	bool test(std::string const &value) const;
};

#endif // SIMPLE_DTD_ATTRIBUTE_HH
