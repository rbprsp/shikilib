#include "catalog.h"
#include "db/db.h"
#include "utils/utils.h"

#include <chrono>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

namespace fs = std::filesystem;

bool IsCached(int page)
{
    std::string file_name = "pages/page_" + std::to_string(page) + ".json";
    return fs::exists(file_name);
}

Catalog::Catalog(AnilibClient &client) : client(client)
{
    seed_url = "https://" + client.Host() +
               "/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5";

    std::string temp_seed = anilib::Utils::GetSeed(client.Get(seed_url).text);
    if (temp_seed != "")
        seed = temp_seed;
    else
        throw std::invalid_argument("Invalid seed value");

    if (!fs::exists("pages"))
        fs::create_directory("pages");
}

void Catalog::ParseNetwork(AnilibCatalog &anilib, int page)
{
    std::string url = "https://" + client.Host() +
                      "/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&page=" +
                      std::to_string(page) + "&seed=" + seed +
                      "&site_id[]=5&sort_by=created_at&sort_type=asc";

    Networker::Response resp = client.Get(url);

    if (resp.code == 429)
    {
        spdlog::warn("Rate limited on page {0}, waiting {1}s", page, rate_timeout);
        std::this_thread::sleep_for(std::chrono::seconds(rate_timeout));
        ParseNetwork(anilib, page);
        return;
    }

    if (resp.code != 200)
    {
        spdlog::critical("HTTP {0} on page {1}", resp.code, page);
        return;
    }

    auto ec = glz::read<glz::opts{.error_on_unknown_keys = false}>(anilib, resp.text);
    if (ec)
    {
        spdlog::critical("Failed to read json");
        spdlog::critical(glz::format_error(ec, resp.text));
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
    if (anilib.data.size() != max_titles)
        ParseNetwork(anilib, page);
}

AnilibCatalog Catalog::ParsePage(int page)
{
    std::string page_data;
    AnilibCatalog ac;

    if (!IsCached(page))
        ParseNetwork(ac, page);
    else
        ParseLocal(ac, page);

    if (ac.data.empty())
        return ac;

    std::string file_name = "pages/page_" + std::to_string(page) + ".json";

    auto ec = glz::write_file_json(ac, file_name, page_data);
    if (ec)
    {
        spdlog::warn("Failed to cache page -> {0}", page);
        spdlog::critical(glz::format_error(ec, page_data));
    }

    seed = ac.meta.seed;

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
                    a.rus_name = item.rus_name.value_or("");
                    a.eng_name = item.eng_name.value_or("");
                    a.slug = item.slug;
                    a.slug_url = item.slug_url;

                    if (const auto* t = std::get_if<Type>(&item.type))
                        a.type = t->label;
                    else
                        a.type = "";

                    storage.replace(a);
                }

                ++page;
            }

            return true;
        });

    spdlog::info("Catalog sync finished");
}
