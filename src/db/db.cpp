#include "db.h"

template <typename T, typename Storage>
BaseDB<T, Storage>::BaseDB(Storage &&storage_instance) : storage(std::move(storage_instance))
{
}

template <typename T, typename Storage> Storage &BaseDB<T, Storage>::GetStorage()
{
    return storage;
}

template <typename T, typename Storage> const Storage &BaseDB<T, Storage>::GetStorage() const
{
    return storage;
}

template class BaseDB<Anime, AnilibStorage>;
template class BaseDB<ShikiList, ShikiStorage>;
template class BaseDB<Mapping, MappingStorage>;
template class BaseDB<Bookmark, BookmarkStorage>;

AnilibDB::AnilibDB(const std::string &db_path)
    : BaseDB(orm::make_storage(
          db_path, orm::make_table(
                       "anime", orm::make_column("id", &Anime::id, orm::primary_key()), orm::make_column("name", &Anime::name),
                       orm::make_column("rus_name", &Anime::rus_name), orm::make_column("eng_name", &Anime::eng_name),
                       orm::make_column("type", &Anime::type), orm::make_column("model", &Anime::model),
                       orm::make_column("slug", &Anime::slug),
                       orm::make_column("slug_url", &Anime::slug_url), orm::make_column("shiki_id", &Anime::shiki_id),
                       orm::make_column("shiki_name", &Anime::shiki_name),
                       orm::make_column("shiki_status", &Anime::shiki_status),
                       orm::make_column("shiki_score", &Anime::shiki_score))))
{
    storage.sync_schema();
}

ShikiDB::ShikiDB(const std::string &db_path)
    : BaseDB(orm::make_storage(
          db_path,
          orm::make_table(
              "shiki_anime", orm::make_column("target_id", &ShikiList::target_id),
              orm::make_column("target_title", &ShikiList::target_title),
              orm::make_column("target_title_ru", &ShikiList::target_title_ru),
              orm::make_column("target_type", &ShikiList::target_type), orm::make_column("score", &ShikiList::score),
              orm::make_column("status", &ShikiList::status), orm::make_column("rewatches", &ShikiList::rewatches),
              orm::make_column("episodes", &ShikiList::episodes), orm::make_column("text", &ShikiList::text))))
{
    storage.sync_schema();
}

MappingDB::MappingDB(const std::string &db_path)
    : BaseDB(
        orm::make_storage
        (
          db_path,
          orm::make_table
          ("mapping",
                          orm::make_column("shiki_id", &Mapping::shiki_id, orm::primary_key()),
                          orm::make_column("anilib_id", &Mapping::anilib_id),
                          orm::make_column("anilist_id", &Mapping::anilist_id),
                          orm::make_column("name", &Mapping::name)
          )
        )
    )
{
    storage.sync_schema();
}

BookmarkDB::BookmarkDB(const std::string &db_path)
    : BaseDB(orm::make_storage(
          db_path,
          orm::make_table("bookmarks",
                          orm::make_column("media_id", &Bookmark::media_id, orm::primary_key()),
                          orm::make_column("status", &Bookmark::status),
                          orm::make_column("created_at", &Bookmark::created_at),
                          orm::make_column("updated_at", &Bookmark::updated_at),
                          orm::make_column("rewatches", &Bookmark::rewatches))))
{
    storage.sync_schema();
}
