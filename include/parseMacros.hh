#ifndef PARSE_MACROS_HH
#define PARSE_MACROS_HH

#include <map>

typedef std::map<std::string, std::string> Macros;

Macros parseMacros(char const *data);

#endif // PARSE_MACROS_HH
