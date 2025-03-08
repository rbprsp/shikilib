#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>

#include "parser.h"

#ifdef __linux__
    //TODO:

#elif _WIN32
    #include <windows.h>
    #include <commdlg.h>

#else
    //TODO:
#endif

nlohmann::json Parser::ReadFile(const std::string file_name)
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

void SaveNotFound(const nlohmann::json& not_found)
{
    if (not_found.empty()) 
    {
        spdlog::info("All anime were found, no data written to not_found.json");
        return;
    }

    std::ofstream file("not_found.json");
    if (file.is_open()) 
    {
        try 
        {
            file << not_found.dump(4);
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

nlohmann::json Parser::MergeLists(const nlohmann::json& animelib_json, const nlohmann::json& shikimori_json)
{
    nlohmann::json result = nlohmann::json::array();
    nlohmann::json not_found = nlohmann::json::array();

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
        } 
        else 
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

                // title
                merged["title"]["en"] = animelib_item.contains("eng_name") && animelib_item["eng_name"].is_string()
                    ? animelib_item["eng_name"].get<std::string>() : "";
                merged["title"]["ru"] = shiki_item.contains("target_title_ru") && shiki_item["target_title_ru"].is_string()
                    ? shiki_item["target_title_ru"].get<std::string>() : "";

                // info
                merged["info"]["anilib_id"] = animelib_item.contains("id") && animelib_item["id"].is_number_integer()
                    ? animelib_item["id"].get<int>() : 0;
                merged["info"]["shiki_id"] = shiki_item.contains("target_id") && shiki_item["target_id"].is_number_integer()
                    ? shiki_item["target_id"].get<int>() : 0;
                merged["info"]["score"] = shiki_item.contains("score") && shiki_item["score"].is_number_integer()
                    ? shiki_item["score"].get<int>() : 0;
                merged["info"]["slug"] = animelib_item.contains("slug") && animelib_item["slug"].is_string()
                    ? animelib_item["slug"].get<std::string>() : "";
                merged["info"]["slug_url"] = animelib_item.contains("slug_url") && animelib_item["slug_url"].is_string()
                    ? animelib_item["slug_url"].get<std::string>() : "";
                merged["info"]["model"] = animelib_item.contains("model") && animelib_item["model"].is_string()
                    ? animelib_item["model"].get<std::string>() : ""; // Добавляем model из AniLib

                // bookmark
                merged["bookmark"]["status_shiki"] = shiki_item.contains("status") && shiki_item["status"].is_string()
                    ? shiki_item["status"].get<std::string>() : "";
                std::string shiki_status = shiki_item.contains("status") && shiki_item["status"].is_string()
                    ? shiki_item["status"].get<std::string>() : "";
                if (shiki_status == "completed") merged["bookmark"]["status_anilib"] = 24;
                else if (shiki_status == "on_hold") merged["bookmark"]["status_anilib"] = 27;
                else if (shiki_status == "rewatching") merged["bookmark"]["status_anilib"] = 26;
                else if (shiki_status == "dropped") merged["bookmark"]["status_anilib"] = 23;
                else if (shiki_status == "watching") merged["bookmark"]["status_anilib"] = 21;
                else if (shiki_status == "planned") merged["bookmark"]["status_anilib"] = 22;
                else merged["bookmark"]["status_anilib"] = 0;
                merged["bookmark"]["progress"] = shiki_item.contains("episodes") && shiki_item["episodes"].is_number_integer()
                    ? shiki_item["episodes"].get<int>() : 0;
                merged["bookmark"]["episodes"] = animelib_item.contains("episodes") && animelib_item["episodes"].is_number_integer()
                    ? animelib_item["episodes"].get<int>() : 0; //ядаун надафикс тут

                // cover
                if (animelib_item.contains("cover") && animelib_item["cover"].is_object()) 
                {
                    merged["cover"]["default"] = animelib_item["cover"].contains("default") && animelib_item["cover"]["default"].is_string()
                        ? animelib_item["cover"]["default"].get<std::string>() : "";
                    merged["cover"]["filename"] = animelib_item["cover"].contains("filename") && animelib_item["cover"]["filename"].is_string()
                        ? animelib_item["cover"]["filename"].get<std::string>() : "";
                    merged["cover"]["md"] = animelib_item["cover"].contains("md") && animelib_item["cover"]["md"].is_string()
                        ? animelib_item["cover"]["md"].get<std::string>() : "";
                    merged["cover"]["thumbnail"] = animelib_item["cover"].contains("thumbnail") && animelib_item["cover"]["thumbnail"].is_string()
                        ? animelib_item["cover"]["thumbnail"].get<std::string>() : "";
                } 
                else 
                {
                    merged["cover"] = nlohmann::json::object();
                }

                result.push_back(merged);
                found = true;
                spdlog::debug("Anime found: {0}", target_title);
                break;
            }
        }

        if (!found) 
        {
            spdlog::debug("Anime not found in animelib: {0}", target_title);
            not_found.push_back(shiki_item);
        }
    }

    SaveNotFound(not_found);
    spdlog::info("Merge completed. Found {0} out of {1} matches", result.size(), total_shikimori);
    return result;
}

nlohmann::json Parser::MergeNotFound(const nlohmann::json& animelib_json, const nlohmann::json& not_found_json)
{
    nlohmann::json result = nlohmann::json::array();
    nlohmann::json still_not_found = nlohmann::json::array();

    if (not_found_json.empty() || !not_found_json.is_array()) 
    {
        spdlog::warn("Not found JSON is empty or not an array");
        return result;
    }

    if (animelib_json.empty() || !animelib_json.is_array()) 
    {
        spdlog::warn("Animelib data is empty or not an array");
        return result;
    }

    size_t total_not_found = not_found_json.size();

    for (const auto& not_found_item : not_found_json) 
    {
        if (not_found_item.empty() || !not_found_item.is_object()) 
        {
            spdlog::warn("Invalid or empty object found in not_found data");
            continue;
        }

        std::string target_title_ru;
        if (not_found_item.contains("target_title_ru") && not_found_item["target_title_ru"].is_string()) 
        {
            target_title_ru = not_found_item["target_title_ru"].get<std::string>();
        } 
        else 
        {
            spdlog::warn("Not found object missing or invalid target_title_ru");
            continue;
        }

        spdlog::debug("Searching for anime by Russian title: {0}", target_title_ru);

        bool found = false;
        for (const auto& animelib_item : animelib_json) 
        {
            if (!animelib_item.is_object() || animelib_item.empty()) 
            {
                spdlog::debug("Skipping invalid or empty object in animelib");
                continue;
            }

            if (animelib_item.contains("rus_name") && animelib_item["rus_name"].is_string() && 
                animelib_item["rus_name"].get<std::string>() == target_title_ru) 
            {
                nlohmann::json merged;

                // title
                merged["title"]["en"] = animelib_item.contains("eng_name") && animelib_item["eng_name"].is_string()
                    ? animelib_item["eng_name"].get<std::string>() : "";
                merged["title"]["ru"] = not_found_item.contains("target_title_ru") && not_found_item["target_title_ru"].is_string()
                    ? not_found_item["target_title_ru"].get<std::string>() : "";

                // info
                merged["info"]["anilib_id"] = animelib_item.contains("id") && animelib_item["id"].is_number_integer()
                    ? animelib_item["id"].get<int>() : 0;
                merged["info"]["shiki_id"] = not_found_item.contains("target_id") && not_found_item["target_id"].is_number_integer()
                    ? not_found_item["target_id"].get<int>() : 0;
                merged["info"]["score"] = not_found_item.contains("score") && not_found_item["score"].is_number_integer()
                    ? not_found_item["score"].get<int>() : 0;
                merged["info"]["slug"] = animelib_item.contains("slug") && animelib_item["slug"].is_string()
                    ? animelib_item["slug"].get<std::string>() : "";
                merged["info"]["slug_url"] = animelib_item.contains("slug_url") && animelib_item["slug_url"].is_string()
                    ? animelib_item["slug_url"].get<std::string>() : "";
                merged["info"]["model"] = animelib_item.contains("model") && animelib_item["model"].is_string()
                    ? animelib_item["model"].get<std::string>() : ""; 

                // bookmark
                merged["bookmark"]["status_shiki"] = not_found_item.contains("status") && not_found_item["status"].is_string()
                    ? not_found_item["status"].get<std::string>() : "";
                std::string shiki_status = not_found_item.contains("status") && not_found_item["status"].is_string()
                    ? not_found_item["status"].get<std::string>() : "";
                if (shiki_status == "completed") merged["bookmark"]["status_anilib"] = 24;
                else if (shiki_status == "on_hold") merged["bookmark"]["status_anilib"] = 27;
                else if (shiki_status == "rewatching") merged["bookmark"]["status_anilib"] = 26;
                else if (shiki_status == "dropped") merged["bookmark"]["status_anilib"] = 23;
                else if (shiki_status == "watching") merged["bookmark"]["status_anilib"] = 21;
                else if (shiki_status == "planned") merged["bookmark"]["status_anilib"] = 22;
                else merged["bookmark"]["status_anilib"] = 0;
                merged["bookmark"]["progress"] = not_found_item.contains("episodes") && not_found_item["episodes"].is_number_integer()
                    ? not_found_item["episodes"].get<int>() : 0;
                merged["bookmark"]["episodes"] = animelib_item.contains("episodes") && animelib_item["episodes"].is_number_integer()
                    ? animelib_item["episodes"].get<int>() : 0; // fix

                // cover
                if (animelib_item.contains("cover") && animelib_item["cover"].is_object()) 
                {
                    merged["cover"]["default"] = animelib_item["cover"].contains("default") && animelib_item["cover"]["default"].is_string()
                        ? animelib_item["cover"]["default"].get<std::string>() : "";
                    merged["cover"]["filename"] = animelib_item["cover"].contains("filename") && animelib_item["cover"]["filename"].is_string()
                        ? animelib_item["cover"]["filename"].get<std::string>() : "";
                    merged["cover"]["md"] = animelib_item["cover"].contains("md") && animelib_item["cover"]["md"].is_string()
                        ? animelib_item["cover"]["md"].get<std::string>() : "";
                    merged["cover"]["thumbnail"] = animelib_item["cover"].contains("thumbnail") && animelib_item["cover"]["thumbnail"].is_string()
                        ? animelib_item["cover"]["thumbnail"].get<std::string>() : "";
                } 
                else 
                {
                    merged["cover"] = nlohmann::json::object();
                }

                result.push_back(merged);
                found = true;
                spdlog::debug("Anime found by Russian title: {0}", target_title_ru);
                break;
            }
        }

        if (!found) 
        {
            spdlog::debug("Anime still not found in animelib by Russian title: {0}", target_title_ru);
            still_not_found.push_back(not_found_item);
        }
    }

    SaveNotFound(still_not_found);
    spdlog::info("MergeNotFound completed. Found {0} out of {1} matches", result.size(), total_not_found);
    return result;
}

void Parser::SaveFile(const std::string file_name, const nlohmann::json& j)
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