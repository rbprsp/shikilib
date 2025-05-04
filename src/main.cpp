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
    //Bearer
    std::string input = "";
    std::cout << "Token: ";
    std::getline(std::cin, input);

    //Fetch
    Networker n;
    n.SetToken(input);
    json fetched = n.FetchAnimeLib();

    //Parse &Merge
    Parser p;
    p.SaveFile("anilib.json", fetched);
    json anilib = p.ReadFile("anilib.json");
    json shiki  = p.ReadFile("shiki.json");
    json merged = p.Merge(anilib, shiki);
    p.SaveFile("shikilib.json", merged);

    return 0;
}