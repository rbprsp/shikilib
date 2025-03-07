#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "parser/parser.h"
#include "networker/networker.h"
#include "utils/utils.h"

int main()
{
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%8l%$] %v");
    spdlog::set_level  (spdlog::level::debug);

    // if(Utils::YesOrNo("Do you want to fetch anilib.me library?"))
    // {
    //     Networker n;
    //     std::string s = "anime.json";
    //     Parser::SaveFile(s, n.FetchAnimeLib());
    // }
    std::string file1 = "anime.json";
    std::string file2 = "Relony_animes.json";

    auto j1 = Parser::ReadFile(file1);
    auto j2 = Parser::ReadFile(file2);

    auto merge = Parser::MergeLists(j1, j2);

    std::string file3 = "anime_merged.json";
    Parser::SaveFile(file3, merge);
    return 0;
}