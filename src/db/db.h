#ifndef __DB_H__
#define __DB_H__
#include "models/anilib.h"
#include "models/mapping.h"
#include "models/shiki.h"
#include "shiki/list/shiki_list.h"

#include <sqlite_orm/sqlite_orm.h>
#include <string>

namespace orm = sqlite_orm;

using AnilibStorage = decltype(orm::make_storage(
    std::declval<std::string>(),
    orm::make_table("anime", orm::make_column("id", &Anime::id), orm::make_column("name", &Anime::name),
                    orm::make_column("rus_name", &Anime::rus_name), orm::make_column("eng_name", &Anime::eng_name),
                    orm::make_column("type", &Anime::type), orm::make_column("slug", &Anime::slug),
                    orm::make_column("slug_url", &Anime::slug_url), orm::make_column("shiki_id", &Anime::shiki_id),
                    orm::make_column("shiki_name", &Anime::shiki_name),
                    orm::make_column("shiki_status", &Anime::shiki_status),
                    orm::make_column("shiki_score", &Anime::shiki_score))));

using ShikiStorage = decltype(orm::make_storage(
    std::declval<std::string>(),
    orm::make_table("anime", orm::make_column("target_id", &ShikiList::target_id),
                    orm::make_column("target_title", &ShikiList::target_title),
                    orm::make_column("target_title_ru", &ShikiList::target_title_ru),
                    orm::make_column("target_type", &ShikiList::target_type),
                    orm::make_column("score", &ShikiList::score), orm::make_column("status", &ShikiList::status),
                    orm::make_column("rewatches", &ShikiList::rewatches),
                    orm::make_column("episodes", &ShikiList::episodes), orm::make_column("text", &ShikiList::text))));

using MappingStorage = decltype(orm::make_storage(std::declval<std::string>(),
                                                  orm::make_table("anime", orm::make_column("name", &Mapping::name),
                                                                  orm::make_column("shiki_id", &Mapping::shiki_id),
                                                                  orm::make_column("anilib_id", &Mapping::anilib_id))));

template <typename T, typename Storage> class BaseDB
{
public:
    explicit BaseDB(Storage &&storage_instance);

    Storage &GetStorage();
    const Storage &GetStorage() const;

protected:
    Storage storage;
};

class AnilibDB : public BaseDB<Anime, AnilibStorage>
{
public:
    explicit AnilibDB(const std::string &db_path);
};

class ShikiDB : public BaseDB<ShikiList, ShikiStorage>
{
public:
    explicit ShikiDB(const std::string &db_path);
};

class MappingDB : public BaseDB<Mapping, MappingStorage>
{
public:
    explicit MappingDB(const std::string &db_path);
};

#endif