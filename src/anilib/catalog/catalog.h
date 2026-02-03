#ifndef __ANILIB_CATALOG_H__
#define __ANILIB_CATALOG_H__

#include "models/anilib.h"

class Catalog
{
private:
    std::string token = "";
    std::string seed = "";
    std::string base_url =
        "https://api.cdnlibs.org/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5";

    /// @brief  Max titles per one page
    const size_t MAX_TITLES = 60;

    /// @brief Ratelimit timeout in seconds
    const int RATE_TIMEOUT = 30;

    void ParseNetwork(AnilibCatalog &anilib, int page);
    void ParseLocal(AnilibCatalog &anilib, int page);
    AnilibCatalog ParsePage(int page);

public:
    Catalog(std::string token);

    void SyncPages();
};

#endif