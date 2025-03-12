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

    Parser p;
    json anilib = p.ReadFile("anilib.json");
    json shiki  = p.ReadFile("shiki.json");
    json merged = p.Merge(anilib["data"], shiki);
    p.SaveFile("shikilib.json", merged);

    return 0;
}