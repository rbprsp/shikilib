#include "db.h"

DB::DB(const std::string &db_path)
    : storage(orm::make_storage(
          db_path,

          orm::make_table("anime", orm::make_column("id", &Anime::id, orm::primary_key()),
                          orm::make_column("name", &Anime::name), orm::make_column("rus_name", &Anime::rus_name),
                          orm::make_column("eng_name", &Anime::eng_name), orm::make_column("type", &Anime::type),
                          orm::make_column("slug", &Anime::slug), orm::make_column("slug_url", &Anime::slug_url),
                          orm::make_column("shiki_id", &Anime::shiki_id),
                          orm::make_column("shiki_name", &Anime::shiki_name))))
{
    storage.sync_schema();
}

DB::Storage &DB::GetStorage()
{
    return storage;
}
