#ifndef ANILIB_AUTH_H
#define ANILIB_AUTH_H

#include "networker/networker.h"

#include <string>

class AnilibClient
{
private:
    std::string token;
    std::string host;
    std::string user_id;

    [[nodiscard]] Networker::Headers BuildHeaders() const;
    void ValidateToken();

public:
    AnilibClient(std::string token, std::string host, std::string user_id);

    [[nodiscard]] Networker::Response Get(const std::string &url) const;
    [[nodiscard]] Networker::Response Post(const std::string &url, const std::string &body) const;

    [[nodiscard]] const std::string &Token() const noexcept;
    [[nodiscard]] const std::string &Host() const noexcept;
    [[nodiscard]] const std::string &UserId() const noexcept;
};

#endif
