#ifndef ANILIB_SYNC_H
#define ANILIB_SYNC_H

#include "anilib/auth/auth.h"
#include "mapping/diff.h"
#include "models/anilib.h"

class AnilibBookmarks
{
private:
    AnilibClient &client;

    /// Ratelimit timeout in seconds
    const int rate_timeout = 10;

    void ParseNetwork(AnilibBookmarksPage &page_data, int page);
    void ParseLocal(AnilibBookmarksPage &page_data, int page);
    AnilibBookmarksPage ParsePage(int page);

    bool PostBookmark(const BookmarkPostItem &item);

public:
    explicit AnilibBookmarks(AnilibClient &client);

    void DumpBookmarks();
    void Sync(const BookmarkDiff &diff, bool dry_run);
};

#endif
