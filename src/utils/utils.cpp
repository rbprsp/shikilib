#include "utils.h"

#include <glaze/glaze.hpp>

std::string anilib::Utils::GetSeed(const std::string& text)
{
    Root root{};
    auto ec = glz::read<glz::opts{.error_on_unknown_keys = false}>(root, text);

    if (!ec)
        return root.meta.seed;

    return "";
}
