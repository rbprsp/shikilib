#ifndef ANILIB_CATALOG_H_
#define ANILIB_CATALOG_H_

#include "anilib/auth/auth.h"
#include "models/anilib.h"

class Catalog
{
private:
    AnilibClient &client;
    std::string seed;
    std::string seed_url;

    /// Max titles per on page
    const size_t max_titles = 60;

    /// Ratelimit timeout in seconds
    const int rate_timeout = 10;

    void ParseNetwork(AnilibCatalog &anilib, int page);
    void ParseLocal(AnilibCatalog &anilib, int page);
    AnilibCatalog ParsePage(int page);

public:
    explicit Catalog(AnilibClient &client);

    void SyncPages();
};

#endif
