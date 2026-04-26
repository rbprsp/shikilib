#include "auth.h"

#include <stdexcept>
#include <utility>

AnilibClient::AnilibClient(std::string token, std::string host) : token(std::move(token)), host(std::move(host))
{
    ValidateToken();
}

void AnilibClient::ValidateToken()
{
    const std::string url = "https://" + host + "/api/anime/7389--shingeki-no-kyojin-anime"; //it's hardcoded link, will work until data change
    Networker::Response resp = Networker::PerformRequest(url, token, host);

    if (resp.code != 200 ||
        !resp.text.contains(R"({"data":{"id":7389,"name":"Shingeki no Kyojin",)")) //same hardcode as above
        throw std::invalid_argument("Invalid token");
}

Networker::Response AnilibClient::Get(const std::string &url) const
{
    return Networker::PerformRequest(url, token, host);
}

const std::string &AnilibClient::Token() const
{
    return token;
}

const std::string &AnilibClient::Host() const
{
    return host;
}
