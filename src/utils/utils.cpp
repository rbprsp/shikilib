#include "utils.h"

bool Utils::YesOrNo(std::string msg)
{
    std::string c {};
    while(true)
    {
        std::cout << msg + "[Y/n]" << std::endl;
        std::cout << ">>";
        std::cin >> c;

        if(c == "Y" || c == "y")
            return true;
        else if (c == "N" || c == "n")
            return false;
        else
            continue;
    }
}