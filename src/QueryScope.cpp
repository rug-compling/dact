#include <sstream>
#include <string>

#include <QSharedPointer>

#include "QueryScope.hh"

QueryScope::QueryScope(QSharedPointer<QueryScope const> parent)
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
        scope = scope->d_parent.data();
    }

    return ss.str();
}
