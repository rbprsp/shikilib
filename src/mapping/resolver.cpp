#include "resolver.h"

#include <algorithm>
#include <cctype>
#include <spdlog/spdlog.h>

MappingResolver::MappingResolver(MappingDB &mapping_db, AnilibDB &anilib_db) : mapping_db(mapping_db)
{
    BuildIndex(anilib_db);
}

void MappingResolver::BuildIndex(AnilibDB &anilib_db)
{
    auto rows = anilib_db.GetStorage().get_all<Anime>();
    name_index.reserve(rows.size() * 3);

    for (const auto &a : rows)
    {
        auto add = [&](const std::string &s)
        {
            if (s.empty())
                return;
            auto key = NormalizeName(s);
            if (key.empty())
                return;
            name_index[key].push_back(a.id);
        };

        add(a.name);
        add(a.eng_name);
        add(a.rus_name);
    }

    spdlog::info("NameIndex built: {} keys from {} anilib rows", name_index.size(), rows.size());
}

std::vector<int> MappingResolver::Lookup(const ShikiList &entry) const
{
    std::vector<std::string> bag;
    if (!entry.target_title.empty())
        bag.push_back(NormalizeName(entry.target_title));
    if (!entry.target_title_ru.empty())
        bag.push_back(NormalizeName(entry.target_title_ru));

    std::vector<int> hits;
    for (const auto &key : bag)
    {
        if (key.empty())
            continue;
        auto it = name_index.find(key);
        if (it == name_index.end())
            continue;

        for (int id : it->second)
            if (std::ranges::find(hits, id) == hits.end())
                hits.push_back(id);
    }
    return hits;
}

MappingResolver::Outcome MappingResolver::ResolveByName(ShikiDB &shiki_db)
{
    Outcome out;
    auto entries = shiki_db.GetStorage().get_all<ShikiList>();
    auto &mstorage = mapping_db.GetStorage();

    mstorage.transaction(
        [&]() -> bool
        {
            for (const auto &entry : entries)
            {
                auto cached = mstorage.get_pointer<Mapping>(entry.target_id);
                if (cached && cached->anilib_id.has_value())
                {
                    out.cached++;
                    continue;
                }

                auto hits = Lookup(entry);

                Mapping m;
                m.shiki_id = entry.target_id;
                m.name = entry.target_title;

                if (hits.empty())
                {
                    out.missed++;
                    out.missed_entries.push_back(entry);
                    spdlog::debug("MISS shiki_id={} '{}'", entry.target_id, entry.target_title);
                }
                else if (hits.size() > 1)
                {
                    out.ambiguous++;
                    out.ambiguous_entries.push_back(entry);
                    spdlog::warn("AMBIGUOUS shiki_id={} '{}' -> {} candidates", entry.target_id,
                                 entry.target_title, hits.size());
                }
                else
                {
                    out.matched++;
                    m.anilib_id = hits.front();
                }

                mstorage.replace(m);
            }
            return true;
        });

    spdlog::info("Resolve done: matched={} ambiguous={} missed={} cached={}", out.matched, out.ambiguous, out.missed,
                 out.cached);
    return out;
}

std::string MappingResolver::NormalizeName(std::string_view name)
{
    std::string normalized_name;
    normalized_name.reserve(name.size());

    for (char c : name)
    {
        auto uc = static_cast<unsigned char>(c);
        if (uc > 127)
            normalized_name += static_cast<char>(uc);
        else if (std::isalnum(uc) != 0)
            normalized_name += static_cast<char>(std::tolower(uc));
    }

    return normalized_name;
}
