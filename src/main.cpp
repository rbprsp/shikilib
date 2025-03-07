#include <iostream>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "parser/parser.h"
#include "networker/networker.h"
#include "utils/utils.h"

int main()
{
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%8l%$] %v");
    spdlog::set_level  (spdlog::level::debug);

#ifdef _WIN32
    system("chcp 65001 >nul");
#endif

    Networker n;
    if(Utils::YesOrNo("Use authentificated requests?"))
    {
        std::cout << "Authorization: ";
        std::string token = ""; //fix here
        std::getline(std::cin, token);
        n.SetToken(token);
    }

    if(Utils::YesOrNo("Do you want to fetch anilib.com?"))
        Parser::SaveFile("anilib.json", n.FetchAnimeLib());

    nlohmann::json anilib = Parser::ReadFile("anilib.json");
    nlohmann::json shiki  = Parser::ReadFile("shiki.json");
    nlohmann::json merged = Parser::MergeLists(anilib, shiki);
    nlohmann::json not_found {};
    if(std::filesystem::exists("not_found.json"))
        not_found = Parser::ReadFile("not_found.json");

    nlohmann::json merged_not_found = Parser::MergeNotFound(anilib, not_found);

    merged.insert(merged.end(), merged_not_found.begin(), merged_not_found.end());

    Parser::SaveFile("shikilib.json", merged);

    spdlog::critical("If there is still some not found anime, please add them manualy, I'll fix them later");
    if(Utils::YesOrNo("Would you like to update your anilib.me profile?"))
    {
        n.AddToAnimeLibFromJson(merged);
        return 0;
    }
    return 0;
}