#ifndef __SHIKI_LIST_H__
#define __SHIKI_LIST_H__

#include "models/shiki.h"

#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <vector>

class ShikiData
{
private:
    std::string file_name = "shiki.json"; // temp
    std::vector<ShikiList> ReadFile();

public:
    void CreateDB();
};

#endif