#ifndef MAPPING_H
#define MAPPING_H

#include <array>
#include <optional>
#include <string_view>
#include <utility>

namespace mapping
{

class Status
{
private:
    static constexpr std::array<std::pair<int, std::string_view>, 7> table
    {{
        {21, "watching"},
        {22, "planned"},
        {23, "dropped"},
        {24, "completed"},
        {25, "completed"}, //some platforms don't have favorites
        {26, "rewatching"},
        {27, "on_hold"}
    }};
public:
    static constexpr std::optional<std::string_view> ToString(int status)
    {
        for (const auto &[num, str] : table)
        {
            if (num == status)
                return str;
        }
        return std::nullopt;
    }

    static constexpr std::optional<int> FromString(std::string_view status)
    {
        if (status == "favorites")
            status = "completed";

        for (const auto &[num, str] : table)
        {
            if (str == status)
                return num;
        }
        return std::nullopt;
    }
};

} // namespace mapping

#endif
