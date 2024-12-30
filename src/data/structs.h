#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <string>

struct anime_shikimori
{
    std::string title;
    std::string title_ru;
    int         id;
    std::string type;
    int         score;
    std::string status;
    int         rewatches;
    int         episodes;
    std::string text;
};

struct anime_animelib
{
    int         id;
    std::string name;
    std::string rus_name;
    std::string eng_name;
    std::string slug;
    std::string slug_url;
    //dont need cover rn, may be implement it later with ui
    //std::vector<std::string> cover;
    //same for age restrictions
    //std::map<int, std::string> age_restriction;
};

struct anime_merged
{
    int         id_shiki;
    int         id_animelib;
    int         score;
    std::string jp_name;
    std::string en_name;
    std::string ru_name;
    std::string slug_url;
    std::string status;
};

#endif //!__STRUCTS_H__