#ifndef QUERY_SCOPE_HH
#define QUERY_SCOPE_HH

#include <string>

#include <QSharedPointer>

class QueryScope
{
public:
    QueryScope(QSharedPointer<QueryScope const> parent);
    QueryScope();

    void setNodeName(std::string const &name);
    std::string const &nodeName() const;
    std::string path() const;
private:
    std::string d_nodeName;
    QSharedPointer<QueryScope const> d_parent;
};

#endif // QUERY_SCOPE_HH
