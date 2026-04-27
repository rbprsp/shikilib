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
