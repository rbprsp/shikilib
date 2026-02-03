#include "catalog.h"
#include "db/db.h"
#include "networker/networker.h"
#include "utils/utils.h"

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
    Networker n;

    std::string token_validate_url = "https://api.cdnlibs.org/api/anime/7389--shingeki-no-kyojin-anime";
    std::string token_response = n.PerformRequest(token_validate_url, token);

    if (token_response.contains("{\"data\":{\"id\":7389,\"name\":\"Shingeki no Kyojin\","))
        this->token = token;
    else
        throw std::invalid_argument("Invalid token");

    std::string _seed = ANILIB::Utils::GetSeed(n.PerformRequest(base_url, token));
    if (_seed != "")
        this->seed = _seed;
    else
        throw std::invalid_argument("Invalid seed value");

    if (!fs::exists("pages"))
        fs::create_directory("pages");
}

void Catalog::ParseNetwork(AnilibCatalog &anilib, int page)
{
    std::string url = "https://api.cdnlibs.org/api/"
                      "anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&page=" +
                      std::to_string(page) + "&seed=" + this->seed + "&site_id[]=5&sort_by=created_at&sort_type=asc";
    Networker n;
    std::string page_data = n.PerformRequest(url, this->token);
    auto ec = glz::read_json(anilib, page_data);
    if (ec)
    {
        std::string formated_error = glz::format_error(ec, page_data);
        if (formated_error.contains("<!DOCTYPE html>"))
        {
            spdlog::warn("Rate limited, please restart app, I'll fix it later!!!");
        }
        else
        {
            spdlog::critical("Failed to read json");
            spdlog::critical(formated_error);
        }
    }
}

void Catalog::ParseLocal(AnilibCatalog &anilib, int page)
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
        ParseNetwork(anilib, page);
}

AnilibCatalog Catalog::ParsePage(int page)
{
    std::string page_data;
    AnilibCatalog ac;

    if (!IsCached(page))
        this->ParseNetwork(ac, page);
    else
        this->ParseLocal(ac, page);

    std::string file_name = "pages/page_" + std::to_string(page) + ".json";

    auto ec = glz::write_file_json(ac, file_name, page_data);
    if (ec)
    {
        spdlog::warn("Failed to cache page -> {0}", page);
        spdlog::critical(glz::format_error(ec, page_data));
    }

    this->seed = ac.meta.seed;

    spdlog::info("Fetching page -> {0}, titles -> {1}", page, ac.data.size());

    return ac;
}

void Catalog::SyncPages()
{
    AnilibDB anilib("anilib.db");
    auto &storage = anilib.GetStorage();
    storage.transaction(
        [&]() -> bool
        {
            int page = 1;

            while (true)
            {
                AnilibCatalog response = ParsePage(page);

                if (response.data.empty())
                    break;

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
