#ifndef NETWORKER_H
#define NETWORKER_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class Networker
{
public:
    enum class Method : std::uint8_t
    {
        Get,
        Post
    };

    using Headers = std::vector<std::pair<std::string, std::string>>;

    struct Response
    {
        int code;
        std::string text;
    };

    static Response Request(Method method, const std::string &url, const Headers &headers = {},
                            const std::string &body = "");

    static Response PerformRequest(const std::string &url, const std::string &token, const std::string &host);
};

#endif
