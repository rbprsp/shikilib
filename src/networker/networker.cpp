#include "networker.h"

#include <cpr/cpr.h>

std::string Networker::PerformRequest(const std::string &url, const std::string &token)
{
    cpr::Response r =
        cpr::Get(cpr::Url{url},
                 cpr::Header{{"Host", "api.cdnlibs.org"},
                             {"User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0"},
                             {"Accept", "*/*"},
                             {"Accept-Language", "en-US,en;q=0.5"},
                             {"Accept-Encoding", "gzip, deflate, br, zstd"},
                             {"Referer", "https://v3.animelib.org"},
                             {"Site-Id", "5"},
                             {"Content-Type", "application/json"},
                             {"Origin", "https://v3.animelib.org"},
                             {"DNT", "1"},
                             {"Sec-GPC", "1"},
                             {"Sec-Fetch-Dest", "empty"},
                             {"Sec-Fetch-Mode", "cors"},
                             {"Sec-Fetch-Site", "cross-site"},
                             {"Connection", "keep-alive"},
                             {"TE", "trailers"},
                             {"Authorization", "Bearer " + token}});

    return r.text;
}