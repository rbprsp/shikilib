#include "networker.h"

#include <cpr/cpr.h>

Networker::Response Networker::Request(Method method, const std::string &url, const Headers &headers,
                                       const std::string &body)
{
    cpr::Header h;
    for (const auto &[k, v] : headers)
        h[k] = v;

    cpr::Response r;
    switch (method)
    {
    case Method::Get:
        r = cpr::Get(cpr::Url{url}, h);
        break;
    case Method::Post:
        r = cpr::Post(cpr::Url{url}, h, cpr::Body{body});
        break;
    }

    return {.code = static_cast<int>(r.status_code), .text = r.text};
}

Networker::Response Networker::PerformRequest(const std::string &url, const std::string &token,
                                              const std::string &host)
{
    Headers h = {{"Host", host},
                 {"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0"},
                 {"Accept", "*/*"},
                 {"Accept-Language", "en-US,en;q=0.5"},
                 {"Accept-Encoding", "gzip, deflate, br, zstd"},
                 {"Referer", "https://animelib.org"},
                 {"Site-Id", "5"},
                 {"Content-Type", "application/json"},
                 {"Origin", "https://animelib.org"},
                 {"DNT", "1"},
                 {"Sec-GPC", "1"},
                 {"Sec-Fetch-Dest", "empty"},
                 {"Sec-Fetch-Mode", "cors"},
                 {"Sec-Fetch-Site", "cross-site"},
                 {"Connection", "keep-alive"},
                 {"TE", "trailers"},
                 {"Authorization", "Bearer " + token}};

    return Request(Method::Get, url, h);
}
