#ifndef SHIKI_LIST_H
#define SHIKI_LIST_H

#include "models/shiki.h"

#include <glaze/glaze.hpp>
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
