#ifndef ANILIB_UTILS_H
#define ANILIB_UTILS_H

#include <string>

struct BaseMeta
{
    std::string seed;
};

struct Root
{
    BaseMeta meta{};
};

namespace anilib
{
    class Utils
    {
    private:
    public:
        static std::string GetSeed(const std::string &text);
    };
} // namespace anilib

#endif
