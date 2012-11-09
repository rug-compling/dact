#ifndef QUERY_SCOPE_HH
#define QUERY_SCOPE_HH

#include <string>
#include <tr1/memory>

class QueryScope
{
public:
    QueryScope(std::tr1::shared_ptr<QueryScope const> parent);
    QueryScope();

    void setNodeName(std::string const &name);
    std::string const &nodeName() const;
    std::string path() const;
private:
    std::string d_nodeName;
    std::tr1::shared_ptr<QueryScope const> d_parent;
};

#endif // QUERY_SCOPE_HH
