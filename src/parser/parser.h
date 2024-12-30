#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <nlohmann/json.hpp>

#include "data/structs.h"

class Parser
{
private:

    std::string file_name    {};
    std::string file_content {};
    bool parse = true;

public:
    void OpenFile();
    void SaveFile(std::vector<nlohmann::json> data, std::string file_name);
    void ParseAnimelib(const nlohmann::json& j, std::vector<anime_animelib>& anime);
    void ParseShikimori(const nlohmann::json& j, anime_shikimori& anime);


    std::vector<anime_merged> MergeLibs(const nlohmann::json& animelib, const nlohmann::json& shiki);
    std::vector<anime_shikimori> ShikiToStruct();
};

#endif //!__PARSER_H__