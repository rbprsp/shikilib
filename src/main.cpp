#include "anilib/auth/auth.h"
#include "anilib/catalog/catalog.h"
#include "anilib/sync/sync.h"
#include "config/config.h"
#include "db/db.h"
#include "mapping/diff.h"
#include "shiki/list/shiki_list.h"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <string>

int main(int argc, char* argv[])
{
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%8l%$] %v");
#ifdef _WIN32
    system("chcp 65001 >nul");
#endif

    const std::string config_path = (argc > 1) ? argv[1] : "config.toml";
    const auto config = ConfigLoader::Load(config_path);

    if(config.logging.level == "info")
        spdlog::set_level(spdlog::level::info);
    else if (config.logging.level == "warn")
        spdlog::set_level(spdlog::level::warn);
    else if (config.logging.level == "error")
        spdlog::set_level(spdlog::level::err);
    else if (config.logging.level == "critical")
        spdlog::set_level(spdlog::level::critical);
    else
        spdlog::set_level(spdlog::level::debug);

    AnilibClient client(config.anilib.api_token, config.anilib.host_url, config.anilib.user_id);

    AnilibBookmarks ab(client);
    ab.DumpBookmarks();

    Catalog c(client);
    c.SyncPages();

    ShikiData sd;
    sd.CreateDB();

    ShikiDB  shiki("shiki.db");
    MappingDB map("mapping.db");
    BookmarkDB bmdb("bookmarks.db");
    AnilibDB anilibdb("anilib.db");

    BookmarkDiff bmd = ComputeBookmarkDiff(shiki, map, bmdb, anilibdb);
    ab.Sync(bmd, false);

    return 0;
}
