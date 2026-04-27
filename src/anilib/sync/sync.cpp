#include "sync.h"
#include "db/db.h"

#include <chrono>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <thread>

namespace fs = std::filesystem;

namespace
{

bool IsCached(int page)
{
    std::string file_name = "bookmarks/page_" + std::to_string(page) + ".json";
    return fs::exists(file_name);
}

} // namespace

AnilibBookmarks::AnilibBookmarks(AnilibClient &client) : client(client)
{
    if (!fs::exists("bookmarks"))
        fs::create_directory("bookmarks");
}

void AnilibBookmarks::ParseNetwork(AnilibBookmarksPage &page_data, int page)
{
    std::string url = "https://" + client.Host() + "/api/bookmarks?page=" + std::to_string(page) +
                      "&sort_by=name&sort_type=desc&status=0&user_id=" + client.UserId();

    Networker::Response resp = client.Get(url);

    if (resp.code == 429)
    {
        spdlog::warn("Rate limited on bookmarks page {0}, waiting {1}s", page, rate_timeout);
        std::this_thread::sleep_for(std::chrono::seconds(rate_timeout));
        ParseNetwork(page_data, page);
        return;
    }

    if (resp.code != 200)
    {
        spdlog::critical("HTTP {0} on bookmarks page {1}", resp.code, page);
        return;
    }

    auto ec = glz::read<glz::opts{.error_on_unknown_keys = false}>(page_data, resp.text);
    if (ec)
    {
        spdlog::critical("Failed to read bookmarks json");
        spdlog::critical(glz::format_error(ec, resp.text));
    }
}

void AnilibBookmarks::ParseLocal(AnilibBookmarksPage &page_data, int page)
{
    std::string file_name = "bookmarks/page_" + std::to_string(page) + ".json";
    std::string buffer;
    auto ec = glz::read_file_json(page_data, file_name, buffer);
    if (ec)
    {
        spdlog::critical("Failed to read bookmarks json");
        spdlog::critical(glz::format_error(ec, buffer));
    }
}

AnilibBookmarksPage AnilibBookmarks::ParsePage(int page)
{
    AnilibBookmarksPage bp;

    if (!IsCached(page))
        ParseNetwork(bp, page);
    else
        ParseLocal(bp, page);

    if (bp.data.empty())
        return bp;

    std::string file_name = "bookmarks/page_" + std::to_string(page) + ".json";
    std::string buffer;

    auto ec = glz::write_file_json(bp, file_name, buffer);
    if (ec)
    {
        spdlog::warn("Failed to cache bookmarks page -> {0}", page);
        spdlog::critical(glz::format_error(ec, buffer));
    }

    spdlog::info("Fetching bookmarks page -> {0}, items -> {1}", page, bp.data.size());
    return bp;
}

bool AnilibBookmarks::PostBookmark(const BookmarkPostItem &item)
{
    const std::string url = "https://" + client.Host() + "/api/bookmarks";

    std::string body = R"({"media_type":")" + item.media_type +
                       R"(","media_slug":")" + item.slug_url +
                       R"(","bookmark":{"status":)" + std::to_string(item.status) +
                       R"(},"meta":{}})";

    Networker::Response resp = client.Post(url, body);

    if (resp.code == 429)
    {
        spdlog::warn("Rate limited on POST bookmark anilib_id={}, waiting {}s", item.anilib_id, rate_timeout);
        std::this_thread::sleep_for(std::chrono::seconds(rate_timeout));
        return PostBookmark(item);
    }

    if (resp.code != 200 && resp.code != 201)
    {
        spdlog::warn("POST bookmark anilib_id={} failed, HTTP {}", item.anilib_id, resp.code);
        return false;
    }

    spdlog::info("POST bookmark anilib_id={} succeed, HTTP {}", item.anilib_id, resp.code);

    return true;
}

void AnilibBookmarks::Sync(const BookmarkDiff &diff, bool dry_run)
{
    spdlog::info("Sync start: dry_run={} to_post={} skipped_no_mapping={} skipped_no_slug={}",
                 dry_run, diff.to_post.size(), diff.skipped_no_mapping.size(), diff.skipped_no_slug.size());

    if (dry_run)
    {
        for (const auto &item : diff.to_post)
            spdlog::info("DRY post anilib_id={} slug={} status={}", item.anilib_id, item.slug_url, item.status);
        return;
    }

    int posted = 0;
    int failed = 0;
    for (const auto &item : diff.to_post)
    {
        (PostBookmark(item) ? posted : failed)++;
    }

    spdlog::info("Sync done: posted={} failed={}", posted, failed);
}

void AnilibBookmarks::DumpBookmarks()
{
    BookmarkDB bookmarks_db("bookmarks.db");
    auto &storage = bookmarks_db.GetStorage();

    storage.transaction(
        [&]() -> bool
        {
            int page = 1;
            int total = 0;

            while (true)
            {
                AnilibBookmarksPage response = ParsePage(page);
                if (response.data.empty())
                    break;

                for (const BookmarkItem &item : response.data)
                {
                    Bookmark b;
                    b.media_id = item.media_id;
                    b.status = item.status;
                    b.created_at = item.created_at;
                    b.updated_at = item.updated_at;
                    b.rewatches = item.meta.rewatches;
                    storage.replace(b);
                    ++total;
                }

                ++page;
            }

            spdlog::info("Bookmarks sync finished, total -> {0}", total);
            return true;
        });
}
