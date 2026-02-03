#ifndef __ANILIB_UTILS_H__
#define __ANILIB_UTILS_H__

#include <string>

struct BaseMeta
{
    std::string seed{};
};

struct Root
{
    BaseMeta meta{};
};

namespace ANILIB
{
    class Utils
    {
    private:
    public:
        static std::string GetSeed(const std::string &text);
    };
} // namespace ANILIB

#endif