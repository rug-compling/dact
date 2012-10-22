#include <sstream>
#include <string>
#include <tr1/memory>

#include "QueryScope.hh"

QueryScope::QueryScope(std::tr1::shared_ptr<QueryScope const> parent)
:
    d_parent(parent)
{}

QueryScope::QueryScope() {}

void QueryScope::setNodeName(std::string const &name)
{
    d_nodeName = name;
}

std::string const &QueryScope::nodeName() const
{
    return d_nodeName;
}

std::string QueryScope::path() const
{
    std::stringstream ss;

    QueryScope const *scope = this;

    while (scope)
    {
        ss << ">" << scope->nodeName();
        scope = scope->d_parent.get();
    }

    return ss.str();
}
