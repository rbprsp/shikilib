#include "resolver.h"

#include <optional>
#include <spdlog/spdlog.h>

MappingResolver::MappingResolver(MappingDB &db) : db(db)
{
}

std::vector<ShikiList> MappingResolver::FilterCacheMisses(const std::vector<ShikiList> &entries)
{
    return {};
}

std::unordered_map<int, int> MappingResolver::ResolveBatchAniList(const std::vector<int> &mal_ids)
{
    return {};
}

std::optional<int> MappingResolver::ResolveAnilibByAnilistId(int anilist_id)
{
    return std::nullopt;
}

std::optional<int> MappingResolver::ResolveAnilibByTitle(const ShikiList &entry)
{
    return std::nullopt;
}

void MappingResolver::Persist(const Mapping &m)
{
    db.Upsert(m);
}

void MappingResolver::ResolveAll(const std::vector<ShikiList> &entries,
                                 const std::function<void(const ResolveProgress &)> &on_tick)
{
}

std::optional<Mapping> MappingResolver::ResolveOne(const ShikiList &entry)
{
    return std::nullopt;
}
