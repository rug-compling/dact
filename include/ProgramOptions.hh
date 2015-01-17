#ifndef PROGRAMOPTIONS_HH
#define PROGRAMOPTIONS_HH

#include <map>
#include <set>
#include <string>
#include <vector>

#include <QtGlobal>

#if defined(Q_WS_WIN)
#include "win_getopt.h"
#else
#include <unistd.h>
#endif

class ProgramOptions
{
public:
    ProgramOptions(int argc, char const *argv[], char const *optString);
    std::vector<std::string> const &arguments() const;
    std::string const &programName() const;
    bool option(char option) const;
    std::string const &optionValue(char option) const;
private:
    std::string d_programName;
    std::map<char, std::string> d_optionValues;
    std::set<char> d_options;
    std::vector<std::string> d_arguments;
};

inline std::vector<std::string> const &ProgramOptions::arguments() const
{
    return d_arguments;
}

inline std::string const &ProgramOptions::programName() const
{
    return d_programName;
}

inline bool ProgramOptions::option(char option) const
{
    return d_options.find(option) != d_options.end();
}

#endif // PROGRAMOPTIONS_HH

