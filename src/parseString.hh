#ifndef PARSE_STRING_HH
#define PARSE_STRING_HH

#include <sstream>
#include <string>
#include <stdexcept>

template <typename T>
T parseString(std::string const &str)
{
    std::istringstream iss(str);
    T val;
    iss >> val;

    if (!iss)
        throw std::invalid_argument("Error parsing value: " + str);

    return val;
}

#endif // PARSE_STRING_HH
