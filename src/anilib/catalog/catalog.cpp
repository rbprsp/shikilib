#include "catalog.h"
#include "anilib/utils/utils.h"
#include "db/db.h"
#include "networker/networker.h"

#include <format>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

bool IsCached(int page)
{
    std::string page_data = "";
    std::string file_name = "pages/page_" + std::to_string(page) + ".json";
    if (fs::exists(file_name))
        return true;
    else
        return false;
}

Catalog::Catalog(std::string token)
{
    this->token = token;
    Networker n;
    std::string _seed = ANILIB::Utils::GetSeed(n.PerformRequest(base_url, token));
    if (_seed != "")
        this->seed = _seed;
    else
        throw std::invalid_argument("Invalid seed value");
}

void Catalog::ParseNetwork(AnilibResponse &anilib, int page)
{
    std::string url = "https://api.cdnlibs.org/api/"
                      "anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&page=" +
                      std::to_string(page) + "&seed=" + this->seed + "&site_id[]=5&sort_by=created_at&sort_type=asc";
    Networker n;
    std::string page_data = n.PerformRequest(url, this->token);
    auto ec = glz::read_json(anilib, page_data);
    if (ec)
    {
        spdlog::critical("Failed to read json");
        spdlog::critical(glz::format_error(ec, page_data));
    }
}

void Catalog::ParseLocal(AnilibResponse &anilib, int page)
{
    std::string file_name = "pages/page_" + std::to_string(page) + ".json";
    std::string page_data;
    auto ec = glz::read_file_json(anilib, file_name, page_data);
    if (ec)
    {
        spdlog::critical("Failed to read json");
        spdlog::critical(glz::format_error(ec, page_data));
    }
    if (anilib.data.size() != MAX_TITLES)
    {
        spdlog::debug("Parsing local page {0} for network updates", page);
        ParseNetwork(anilib, page); // fetch local page once again for updates?
    }
}

AnilibResponse Catalog::ParsePage(int page)
{
    std::string page_data;
    AnilibResponse ar;

    if (!IsCached(page))
        this->ParseNetwork(ar, page);
    else
        this->ParseLocal(ar, page);

    std::string file_name = "pages/page_" + std::to_string(page) + ".json";

    auto ec = glz::write_file_json(ar, file_name, page_data);
    if (ec)
        spdlog::warn("Failed to cache page -> {0}", page);

    this->seed = ar.meta.seed;

    spdlog::info("Fetching page -> {0}, titles -> {1}", page, ar.data.size());

    return ar;
}

std::vector<AnilibResponse> Catalog::ParsePages()
{
    std::vector<AnilibResponse> result;
    int page = 1;
    int data_size = MAX_TITLES;
    while (data_size == MAX_TITLES)
    {
        result.push_back(this->ParsePage(page));
        page++;
        data_size = result.back().data.size();
    }
    return result;
}

void Catalog::SyncPages()
{
    DB db("animelib.db");
    auto &storage = db.GetStorage();

    storage.transaction(
        [&]() -> bool
        {
            int page = 1;

            while (true)
            {
                AnilibResponse response = ParsePage(page);

                if (response.data.empty())
                {
                    break;
                }

                for (const AnimeItem &item : response.data)
                {
                    Anime a;

                    a.id = item.id;
                    a.name = item.name;
                    a.rus_name = item.rus_name;
                    a.eng_name = item.eng_name.value_or("");
                    a.slug = item.slug;
                    a.slug_url = item.slug_url;
                    a.type = item.type.label;

                    storage.replace(a);
                }

                ++page;
            }

            return true;
        });

    spdlog::info("Catalog sync finished");
}
