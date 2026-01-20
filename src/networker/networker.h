#ifndef __NETWORKER_H__
#define __NETWORKER_H__

#include <string>

class Networker
{
private:
public:
    static std::string PerformRequest(const std::string &url, const std::string &token);
};

#endif