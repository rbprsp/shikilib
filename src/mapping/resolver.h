#ifndef MAPPING_RESOLVER_H
#define MAPPING_RESOLVER_H

#include "db/db.h"
#include "models/mapping.h"
#include "models/shiki.h"

#include <atomic>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

struct ResolveProgress
{
    std::atomic<int> done{0};
    std::atomic<int> total{0};
    std::atomic<int> resolved{0};
    std::atomic<int> failed{0};
    std::atomic<bool> cancel{false};
};

class MappingResolver
{
public:
    explicit MappingResolver(MappingDB &db);

    void ResolveAll(const std::vector<ShikiList> &entries,
                    const std::function<void(const ResolveProgress &)> &on_tick = {});

    std::optional<Mapping> ResolveOne(const ShikiList &entry);

private:
    MappingDB &db;
    ResolveProgress progress;

    std::vector<ShikiList> FilterCacheMisses(const std::vector<ShikiList> &entries);
    std::unordered_map<int, int> ResolveBatchAniList(const std::vector<int> &mal_ids);
    std::optional<int> ResolveAnilibByAnilistId(int anilist_id);
    std::optional<int> ResolveAnilibByTitle(const ShikiList &entry);
    void Persist(const Mapping &m);
};

#endif
