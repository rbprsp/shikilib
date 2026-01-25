#pragma once

#include <sqlite_orm/sqlite_orm.h>
#include <string>

namespace orm = sqlite_orm;

struct Anime
{
    int id = 0;

    std::string name;
    std::string rus_name;
    std::string eng_name;
    std::string type;
    std::string slug;
    std::string slug_url;
    std::optional<int> shiki_id;
    std::optional<std::string> shiki_name;
};

class DB
{
public:
    explicit DB(const std::string &db_path);

    using Storage = decltype(orm::make_storage(
        std::declval<std::string>(),

        orm::make_table("anime", orm::make_column("id", &Anime::id, orm::primary_key()),
                        orm::make_column("name", &Anime::name), orm::make_column("rus_name", &Anime::rus_name),
                        orm::make_column("eng_name", &Anime::eng_name), orm::make_column("type", &Anime::type),
                        orm::make_column("slug", &Anime::slug), orm::make_column("slug_url", &Anime::slug_url),
                        orm::make_column("shiki_id", &Anime::shiki_id),
                        orm::make_column("shiki_name", &Anime::shiki_name))));

    Storage &GetStorage();

private:
    Storage storage;
};
