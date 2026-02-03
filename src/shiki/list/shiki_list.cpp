#include "shiki_list.h"
#include "db/db.h"

#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

std::vector<ShikiList> ShikiData::ReadFile()
{
    std::string file_data;
    std::vector<ShikiList> data;
    auto ec = glz::read_file_json(data, this->file_name, file_data);

    if (ec)
    {
        spdlog::error("Failed to read shiki data");
        spdlog::error(glz::format_error(ec, file_data));
        return {};
    }
    else
        return data;
}

// diff from <anilib/catalog/catalog.cpp>
void ShikiData::CreateDB()
{
    std::vector<ShikiList> shiki_list = this->ReadFile();

    ShikiDB shiki("shiki.db");
    auto &storage = shiki.GetStorage();

    storage.begin_transaction();

    try
    {
        storage.remove_all<ShikiList>();

        for (const auto &item : shiki_list)
            storage.insert(item);

        storage.commit();
    }
    catch (...)
    {
        storage.rollback();
        throw;
    }
}
