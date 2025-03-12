#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <atomic>

#include <nlohmann/json.hpp>

using nlohmann::json;

class Parser
{
private:
    json missing = json::array();

    size_t shiki_size{};
    size_t anilib_size{};

    std::atomic_bool is_found = false;

    json MainMerge   (const json &anilib, const json &shiki);
    json MissingMerge(const json &anilib);

    void SearchDefault(bool from_start, const std::string &target, const json &anilib, json &result);
    void SearchRussian(bool from_start, const std::string &target, const json &anilib, json &result);
    void SearchEnglish(bool from_start, const std::string &target, const json &anilib, json &result);

    json GenerateMergedObject(const json &shiki, const json &anilib);

public:

    /*
        May be I'll implement sax-parsing here in the future, 
        but for now It's using ~150 Mb of RAM


                        else if (shiki_status == "on_hold") merged["bookmark"]["status_anilib"] = 27;
                else if (shiki_status == "rewatching") merged["bookmark"]["status_anilib"] = 26;
                else if (shiki_status == "dropped") merged["bookmark"]["status_anilib"] = 23;
                else if (shiki_status == "watching") merged["bookmark"]["status_anilib"] = 21;
                else if (shiki_status == "planned") merged["bookmark"]["status_anilib"] = 22;


    */

    json ReadFile(const std::string file_name);
    void SaveFile(const std::string file_name, const json& j);

    json Merge(const json& animelib_json, const json& shikimori_json);
};

#endif //!__PARSER_H__