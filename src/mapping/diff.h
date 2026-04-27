#ifndef MAPPING_DIFF_H
#define MAPPING_DIFF_H

#include "db/db.h"

#include <string>
#include <vector>

struct BookmarkPostItem
{
    int anilib_id{};
    std::string slug_url;
    std::string media_type;
    int status{};
};

struct BookmarkDiff
{
    std::vector<BookmarkPostItem> to_post;
    std::vector<int> skipped_no_mapping;
    std::vector<int> skipped_no_slug;
};

BookmarkDiff ComputeBookmarkDiff(ShikiDB &shiki_db, MappingDB &mapping_db, BookmarkDB &bookmark_db,
                                 AnilibDB &anilib_db);

#endif
