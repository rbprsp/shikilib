#ifndef ANILIB_AUTH_H
#define ANILIB_AUTH_H

#include "networker/networker.h"

#include <string>

class AnilibClient
{
private:
    std::string token;
    std::string host;

    void ValidateToken();

public:
    AnilibClient(std::string token, std::string host);

    Networker::Response Get(const std::string &url) const;

    const std::string &Token() const;
    const std::string &Host() const;
};

#endif
