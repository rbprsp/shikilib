#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <atomic>
#include <future>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

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

    json GenerateMergedObject(const json &shiki, const json &anilib);

    json SearchDefault(bool from_start, const std::string &target, const json &anilib);
    json SearchRussian(bool from_start, const std::string &target, const json &anilib);
    json SearchEnglish(bool from_start, const std::string &target, const json &anilib);

public:
    json ReadFile(const std::string file_name);
    void SaveFile(const std::string file_name, const json& j);

    json Merge(const json& animelib_json, const json& shikimori_json);
};

#endif //!__PARSER_H__