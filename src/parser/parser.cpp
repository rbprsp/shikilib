#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>

#include "parser.h"
#include "data/structs.h"

#ifdef __linux__
    //TODO:

#elif _WIN32
    #include <windows.h>
    #include <commdlg.h>

#else
    //TODO:
#endif

nlohmann::json Parser::ReadFile(const std::string& file_name)
{
    nlohmann::json result = {};

    std::ifstream file(file_name);
    if (!file.is_open()) 
    {
        spdlog::error("Failed to open file: {0}", file_name);
        return result;
    }

    try 
    {
        file >> result;
        spdlog::info("Successfully read JSON from file: {0}", file_name);
    }
    catch (const nlohmann::json::parse_error& e) 
    {
        spdlog::error("JSON parse error in file {0}: {1} at byte {2}", 
                      file_name, e.what(), e.byte);
        result = {};
    }
    catch (const std::exception& e) 
    {
        spdlog::error("Unexpected error while reading file {0}: {1}", 
                      file_name, e.what());
        result = {};
    }
    return result;
}

/// @brief using two json files to merge it
/// @param animelib animelib json data
/// @param shiki shikomori json data
/// @returns returns merged json list with your anime from shikimori
nlohmann::json Parser::MergeLists(const nlohmann::json& animelib_json, const nlohmann::json& shikimori_json)
{
    nlohmann::json result = nlohmann::json::array();      // Результирующий JSON-массив для найденных аниме
    nlohmann::json not_found = nlohmann::json::array();   // JSON-массив для ненайденных аниме

    if (shikimori_json.empty() || !shikimori_json.is_array()) 
    {
        spdlog::warn("Shikimori data is empty or not an array");
        return result;
    }

    if (animelib_json.empty() || !animelib_json.is_array()) 
    {
        spdlog::warn("Animelib data is empty or not an array");
        return result;
    }

    size_t total_shikimori = shikimori_json.size();

    for (const auto& shiki_item : shikimori_json) 
    {
        if (shiki_item.empty() || !shiki_item.is_object()) 
        {
            spdlog::warn("Invalid or empty object found in shikimori data");
            continue;
        }

        std::string target_title;
        if (shiki_item.contains("target_title") && shiki_item["target_title"].is_string()) 
        {
            target_title = shiki_item["target_title"].get<std::string>();
        } else 
        {
            spdlog::warn("Shikimori object missing or invalid target_title");
            continue;
        }

        spdlog::debug("Searching for anime: {0}", target_title);

        bool found = false;
        for (const auto& animelib_item : animelib_json) 
        {
            if (!animelib_item.is_object() || animelib_item.empty()) 
            {
                spdlog::debug("Skipping invalid or empty object in animelib");
                continue;
            }

            if (animelib_item.contains("name") && animelib_item["name"].is_string() && 
                animelib_item["name"].get<std::string>() == target_title) 
            {
                nlohmann::json merged;

                merged["id_shiki"] = shiki_item.contains("target_id") && shiki_item["target_id"].is_number_integer()
                    ? shiki_item["target_id"].get<int>() : 0;
                merged["score"] = shiki_item.contains("score") && shiki_item["score"].is_number_integer()
                    ? shiki_item["score"].get<int>() : 0;
                merged["jp_name"] = target_title;
                merged["ru_name"] = shiki_item.contains("target_title_ru") && shiki_item["target_title_ru"].is_string()
                    ? shiki_item["target_title_ru"].get<std::string>() : "";

                merged["id_animelib"] = animelib_item.contains("id") && animelib_item["id"].is_number_integer()
                    ? animelib_item["id"].get<int>() : 0;
                merged["en_name"] = animelib_item.contains("eng_name") && animelib_item["eng_name"].is_string()
                    ? animelib_item["eng_name"].get<std::string>() : "";
                merged["slug_url"] = animelib_item.contains("slug_url") && animelib_item["slug_url"].is_string()
                    ? animelib_item["slug_url"].get<std::string>() : "";
                merged["status"] = animelib_item.contains("status") && animelib_item["status"].is_object() &&
                                 animelib_item["status"].contains("label") && animelib_item["status"]["label"].is_string()
                    ? animelib_item["status"]["label"].get<std::string>() : "";

                result.push_back(merged);
                found = true;
                spdlog::debug("Anime found: {0}", target_title);
                break;
            }
        }

        if (!found) 
        {
            spdlog::debug("Anime not found in animelib: {0}", target_title);
            not_found.push_back(shiki_item); // Добавляем весь объект shikimori_json в not_found
        }
    }

    // Записываем ненайденные аниме в файл
    if (!not_found.empty()) 
    {
        std::ofstream file("not_found.json");
        if (file.is_open()) 
        {
            try 
            {
                file << not_found.dump(4); // Записываем с отступами для читаемости
                spdlog::info("Saved {0} not found anime to not_found.json", not_found.size());
                file.close();
            } 
            catch (const std::exception& e) 
            {
                spdlog::error("Failed to write not_found.json: {0}", e.what());
                file.close();
            }
        } 
        else 
        {
            spdlog::error("Failed to open not_found.json for writing");
        }
    } 
    else 
    {
        spdlog::info("All anime were found, no data written to not_found.json");
    }

    spdlog::info("Merge completed. Found {0} out of {1} matches", result.size(), total_shikimori);
    return result;
}

void Parser::SaveFile(const std::string& file_name, const nlohmann::json& j)
{
    if (file_name.empty()) 
    {
        spdlog::error("Cannot save JSON: file name is empty");
        return;
    }

    if (j.is_null()) 
    {
        spdlog::warn("JSON data is null, saving empty file to {0}", file_name);
    }

    std::ofstream file(file_name);
    if (!file.is_open()) 
    {
        spdlog::error("Failed to open file for writing: {0}", file_name);
        return;
    }

    try 
    {
        file << j.dump(4);
        spdlog::info("Successfully saved JSON to file: {0} (size: {1} bytes)", 
                     file_name, j.dump().size());
    }
    catch (const nlohmann::json::exception& e) 
    {
        spdlog::error("JSON serialization error while saving to {0}: {1}", 
                      file_name, e.what());
    }
    catch (const std::ios_base::failure& e) 
    {
        spdlog::error("I/O error while writing to file {0}: {1}", 
                      file_name, e.what());
    }
    catch (const std::exception& e) 
    {
        spdlog::error("Unexpected error while saving to file {0}: {1}", 
                      file_name, e.what());
    }
}