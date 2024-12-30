#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>

#include "parser.h"

#ifdef __linux__
    //TODO:

#elif _WIN32
    #include <windows.h>
    #include <commdlg.h>

#else
    //TODO:
#endif

void Parser::OpenFile()
{
    #ifdef __linux__
    //TODO:

    #elif _WIN32
        OPENFILENAME ofn;
        CHAR file_name[MAX_PATH] = {};
        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner   = NULL;
        ofn.lpstrFilter = "JSON files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile   = file_name;
        ofn.nMaxFile    = MAX_PATH;
        ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = "json";

        if(GetOpenFileNameA(&ofn))
        {
            this->file_name = file_name;
            spdlog::debug("filename -> {0}", this->file_name);
        }
        else
            spdlog::warn("No files was choosen");

    #else
        //TODO:
    #endif
}

void Parser::ParseShikimori(const nlohmann::json& j, anime_shikimori& anime)
{
    anime.title = j.at("target_title").get<std::string>();
    anime.title_ru = j.at("target_title_ru").get<std::string>();
    anime.id = j.at("target_id").get<int>();
    anime.type = j.at("target_type").get<std::string>();
    anime.score = j.at("score").get<int>();
    anime.status = j.at("status").get<std::string>();
    anime.rewatches = j.at("rewatches").get<int>();
    anime.episodes = j.at("episodes").get<int>();
    anime.text = j.contains("text") && !j.at("text").is_null() ? j.at("text").get<std::string>() : "";
}

void Parser::ParseAnimelib(const nlohmann::json& j, std::vector<anime_animelib>& anime)
{
    if(j.at("data").empty())
    {
        this->parse = false;
        return;
    }
    for(const auto& object : j.at("data"))
    {
        anime_animelib al;
        al.id       = object.at("id").get<int>();
        al.name     = object.at("name").get<std::string>();

        al.rus_name = object.contains("rus_name") && !object["rus_name"].is_null() ? object.at("rus_name").get<std::string>() : "";
        al.eng_name = object.contains("eng_name") && !object["eng_name"].is_null() ? object.at("eng_name").get<std::string>() : "";
        al.slug =     object.contains("slug")     && !object["slug"].is_null()     ? object.at("slug").get<std::string>() : "";
        al.slug_url = object.contains("slug_url") && !object["slug_url"].is_null() ? object.at("slug_url").get<std::string>() : "";
        anime.push_back(al);
    }
}

std::vector<anime_shikimori> Parser::ShikiToStruct() 
{
    std::vector<anime_shikimori> animes;

    std::ifstream file(this->file_name);
    if (!file) 
    {
        spdlog::error("Unable to open file");
        return animes;
    }
    else
    {
        try
        {
            nlohmann::json j_anime;
            file >> j_anime;

            file.close();

            for(const auto& entry : j_anime)
            {
                anime_shikimori anime;
                this->ParseShikimori(entry, anime);
                animes.push_back(anime);
            }
        }
        catch(const std::exception& e)
        {
            spdlog::error("Error: {0}", e.what());
        }
    }
    return animes;
}

/// @brief using two json files to merge it
/// @param animelib animelib json data
/// @param shiki shikomori json data
/// @returns returns merged json list with your anime from shikimori
std::vector<anime_merged> Parser::MergeLibs(const nlohmann::json& animelib, const nlohmann::json& shiki)
{
    std::vector<anime_merged> hh;
    return hh;
}

void Parser::SaveFile(std::vector<nlohmann::json> data, std::string file_name)
{
    nlohmann::ordered_json db = nlohmann::json::array();

    for(const auto& page : data)
    {
        for(const auto& anime : page["data"])
        {
            db.push_back(anime);
        }
    }

    std::ofstream file(file_name);
    if (!file) 
    {
        spdlog::error("Unable to create file -> {0}", file_name);
        return;
    }

    file << db.dump(4);
    file.close();

    spdlog::info("File -> {0} saved", file_name);
}