#include "SimpleDTDAttribute.hh"
#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;

SimpleDTDAttribute::~SimpleDTDAttribute()
{
	// 
}

void SimpleDTDEnumerationAttribute::addValue(std::string const &value)
{
	d_values.insert(value);
}

bool SimpleDTDEnumerationAttribute::test(std::string const &value) const
{
	return d_values.find(value) != d_values.end();
}

bool SimpleDTDNameTokenAttribute::test(std::string const &value) const
{
	return !contains(trim_copy(value), std::string(" "));
}

bool SimpleDTDCDATAAttribute::test(std::string const &value) const 
{
	return true;
}
