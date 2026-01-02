#include <iostream>
#include <fstream>
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

    std::string token = "";
    std::ifstream token_file("token.txt");

    if (token_file.is_open())
    {
        std::getline(token_file, token);
        token_file.close();
        if (!token.empty())
        {
            size_t first = token.find_first_not_of(" \t\n\r");
            if (first != std::string::npos)
                token.erase(0, first);

            size_t last = token.find_last_not_of(" \t\n\r");
            if (last != std::string::npos)
                token.erase(last + 1);

            if (token.find("Bearer ") == 0)
                token.erase(0, 7);
        }
        if (token.empty())
        {
            spdlog::error("Token file is empty!");
            return 1;
        }
        spdlog::info("Bearer token loaded from token.txt");
    }
    else
    {
        spdlog::error("Failed to open token.txt file!");
        spdlog::error("Please create token.txt file with your Bearer token");
        return 1;
    }

    Networker n;
    n.SetToken(token);
    Parser p;


    int input = 0;
    while(input != 9)
    {
        std::cout << "1. Fetch anilib titles" << std::endl;
        std::cout << "2. Fetch anilib bookmarks"  << std::endl;
        std::cout << "3. Merge shiki & anilib bookmarks" << std::endl;
        std::cout << "4. Get not synced" << std::endl;
        std::cout << "5. Add bookmarks to anilib" << std::endl;
        std::cout << "9. Exit" << std::endl;

        std::cout << ">> ";
        std::cin >> input;
        
        switch (input)
        {
        case 1:
            p.SaveFile("anilib.json", n.FetchAnimeLib());
            break;
        case 2:
            p.SaveFile("anilibmarks.json", n.FetchBookmarks());
            break;
        case 3:
            p.SaveFile("shikilib.json", p.Merge(p.ReadFile("anilib.json"), p.ReadFile("shiki.json")));
            break;
        case 4:
            p.SaveFile("notsynced.json", p.GetNotSynced(p.ReadFile("shikilib.json"), p.ReadFile("anilibmarks.json")));
            break;
        case 5:
            n.AddToAnimeLibFromJson(p.ReadFile("notsynced.json"));
            break;
        default:
            break;
        }
    }
    
    return 0;
}