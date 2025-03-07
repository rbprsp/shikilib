#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <nlohmann/json.hpp>

#include "data/structs.h"

class Parser
{
public:
    static nlohmann::json ReadFile(const std::string file_name);
    static void           SaveFile(const std::string file_name, const nlohmann::json& j);

    static nlohmann::json MergeLists(const nlohmann::json& animelib_json, const nlohmann::json& shikimori_json);
    static nlohmann::json MergeNotFound(const nlohmann::json& animelib_json, const nlohmann::json& not_found_json);
};

#endif //!__PARSER_H__