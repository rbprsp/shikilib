#include <iostream>
#include <spdlog/spdlog.h>

#include "parser/parser.h"
#include "networker/networker.h"

int main()
{
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%8l%$] %v");
    spdlog::set_level  (spdlog::level::debug);


    spdlog::info("Please choose .json file from shikimori");
    spdlog::info("Press any key to open file dialog");
    
    std::cin.get();

    Parser* p = new Parser;
    p->OpenFile();



    return 0;
}