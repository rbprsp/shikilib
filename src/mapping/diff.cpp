#include "diff.h"
#include "mapping.h"
#include "models/shiki.h"

#include <spdlog/spdlog.h>

BookmarkDiff ComputeBookmarkDiff(ShikiDB &shiki_db, MappingDB &mapping_db, BookmarkDB &bookmark_db,
                                 AnilibDB &anilib_db)
{
    BookmarkDiff diff;

    auto entries = shiki_db.GetStorage().get_all<ShikiList>();
    auto &mstorage = mapping_db.GetStorage();
    auto &bstorage = bookmark_db.GetStorage();
    auto &astorage = anilib_db.GetStorage();

    for (const auto &entry : entries)
    {
        auto map_row = mstorage.get_pointer<Mapping>(entry.target_id);
        if (!map_row || !map_row->anilib_id.has_value())
        {
            diff.skipped_no_mapping.push_back(entry.target_id);
            continue;
        }

        int anilib_id = map_row->anilib_id.value();

        auto status_code = mapping::Status::FromString(entry.status);
        if (!status_code.has_value())
        {
            spdlog::warn("Unknown shiki status '{}' for shiki_id={}", entry.status, entry.target_id);
            continue;
        }

        auto anime_row = astorage.get_pointer<Anime>(anilib_id);
        if (!anime_row || anime_row->slug_url.empty() || anime_row->model.empty())
        {
            diff.skipped_no_slug.push_back(anilib_id);
            continue;
        }

        auto existing = bstorage.get_pointer<Bookmark>(anilib_id);
        if (existing && existing->status == status_code.value())
            continue;

        if (existing)
            spdlog::warn("Status conflict anilib_id={} shiki={} anilib={}, shiki wins",
                         anilib_id, status_code.value(), existing->status);

        diff.to_post.push_back({.anilib_id = anilib_id,
                                .slug_url = anime_row->slug_url,
                                .media_type = anime_row->model,
                                .status = status_code.value()});
    }

    spdlog::info("Diff: to_post={} skipped_no_mapping={} skipped_no_slug={}",
                 diff.to_post.size(), diff.skipped_no_mapping.size(), diff.skipped_no_slug.size());
    return diff;
}
