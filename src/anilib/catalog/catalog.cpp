#include "catalog.h"
#include "anilib/utils/utils.h"
#include "networker/networker.h"

#include <format>
#include <fstream>
#include <iostream>

#include <spdlog/spdlog.h>

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

AnilibResponse Catalog::ParsePage(int page)
{
    AnilibResponse ar;

    std::string url = "https://api.cdnlibs.org/api/"
                      "anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&page=" +
                      std::to_string(page) + "&seed=" + this->seed + "&site_id[]=5&sort_by=created_at&sort_type=asc";
    Networker n;
    std::string response = n.PerformRequest(url, this->token);
    auto ec = glz::read_json(ar, response);
    if (ec)
    {
        spdlog::critical("Failed to read json");
        spdlog::critical(glz::format_error(ec, response));
    }

    this->seed = ar.meta.seed;
    spdlog::info("SEED -> {0}", this->seed);
    std::string filename = std::format("page_{:03d}.json", page);

    ec = glz::write_file_json(ar, filename, std::string{});

    for (auto title : ar.data)
        spdlog::info(title.id);

    return ar;
}