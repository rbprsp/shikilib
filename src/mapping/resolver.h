#ifndef MAPPING_RESOLVER_H
#define MAPPING_RESOLVER_H

#include "db/db.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class MappingResolver
{
public:
    struct Outcome
    {
        int matched{0};
        int ambiguous{0};
        int missed{0};
        int cached{0};
        std::vector<ShikiList> missed_entries;
        std::vector<ShikiList> ambiguous_entries;
    };

    MappingResolver(MappingDB &mapping_db, AnilibDB &anilib_db);

    Outcome ResolveByName(ShikiDB &shiki_db);

    static std::string NormalizeName(std::string_view name);

private:
    MappingDB &mapping_db;
    std::unordered_map<std::string, std::vector<int>> name_index;

    void BuildIndex(AnilibDB &anilib_db);
    std::vector<int> Lookup(const ShikiList &entry) const;
};

#endif
