#ifndef MAPPING_MODEL_H
#define MAPPING_MODEL_H

#include <optional>
#include <string>

struct Mapping
{
    int shiki_id{};
    std::optional<int> anilib_id;
    std::optional<int> anilist_id;
    std::string name;
    float confidence{0};
    bool verified{false};
    std::string created_at;
};

#endif
