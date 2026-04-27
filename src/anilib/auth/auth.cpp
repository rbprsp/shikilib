#include "auth.h"

#include <stdexcept>
#include <string_view>
#include <utility>

namespace
{
    constexpr std::string_view site_origin = "https://animelib.org";
}

AnilibClient::AnilibClient(std::string token, std::string host, std::string user_id)
    : token(std::move(token)), host(std::move(host)), user_id(std::move(user_id))
{
    ValidateToken();
}

Networker::Headers AnilibClient::BuildHeaders() const
{
    return {{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0"},
            {"Accept", "*/*"},
            {"Accept-Language", "en-US,en;q=0.5"},
            {"Accept-Encoding", "gzip, deflate, br, zstd"},
            {"Referer", std::string(site_origin)},
            {"Site-Id", "5"},
            {"Content-Type", "application/json"},
            {"Origin", std::string(site_origin)},
            {"DNT", "1"},
            {"Sec-GPC", "1"},
            {"Sec-Fetch-Dest", "empty"},
            {"Sec-Fetch-Mode", "cors"},
            {"Sec-Fetch-Site", "cross-site"},
            {"Connection", "keep-alive"},
            {"TE", "trailers"},
            {"Authorization", "Bearer " + token}};
}

void AnilibClient::ValidateToken()
{
    const std::string url = "https://" + host + "/api/anime/7389--shingeki-no-kyojin-anime";
    Networker::Response resp = Get(url);

    if (resp.code != 200 ||
        !resp.text.contains(R"({"data":{"id":7389,"name":"Shingeki no Kyojin",)"))
        throw std::invalid_argument("Invalid token");
}

Networker::Response AnilibClient::Get(const std::string &url) const
{
    return Networker::Request(Networker::Method::Get, url, BuildHeaders());
}

Networker::Response AnilibClient::Post(const std::string &url, const std::string &body) const
{
    return Networker::Request(Networker::Method::Post, url, BuildHeaders(), body);
}

const std::string &AnilibClient::Token() const noexcept
{
    return token;
}

const std::string &AnilibClient::Host() const noexcept
{
    return host;
}

const std::string &AnilibClient::UserId() const noexcept
{
    return user_id;
}
