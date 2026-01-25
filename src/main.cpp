#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "anilib/catalog/catalog.h"
#include "anilib/utils/utils.h"
#include "networker/networker.h"
#include "parser/parser.h"

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%8l%$] %v");
    spdlog::set_level(spdlog::level::debug);

#ifdef _WIN32
    system("chcp 65001 >nul");
#endif

    Networker n;
    std::string token;

    std::ifstream file("token");

    file >> token;

    std::string url =
        "https://api.cdnlibs.org/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5";

    std::string r = n.PerformRequest(url, token);

    Catalog c(token);

    c.SyncPages();

    return 0;
}