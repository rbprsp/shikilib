#ifndef __SHIKI_MODELS_H__
#define __SHIKI_MODELS_H__

#include <glaze/glaze.hpp>
#include <optional>
#include <string>

struct ShikiList
{
    std::string target_title;
    std::string target_title_ru;
    int target_id{};
    std::string target_type;

    int score{};
    std::string status;
    int rewatches{};
    int episodes{};
    std::optional<std::string> text;

    struct glaze
    {
        using T = ShikiList;

        static constexpr auto value =
            glz::object("target_title", &T::target_title, "target_title_ru", &T::target_title_ru, "target_id",
                        &T::target_id, "target_type", &T::target_type, "score", &T::score, "status", &T::status,
                        "rewatches", &T::rewatches, "episodes", &T::episodes, "text", &T::text);
    };
};

#endif