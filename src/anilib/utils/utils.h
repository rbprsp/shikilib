#ifndef __ANILIB_UTILS_H__
#define __ANILIB_UTILS_H__

#include <string>

struct Meta
{
    std::string seed{};
};

struct Root
{
    Meta meta{};
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