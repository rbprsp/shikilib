#include "parser.h"
#include <iostream>
#include <fstream>

json Parser::Merge(const json& animelib_json, const json& shikimori_json)
{
    json main = MainMerge(animelib_json, shikimori_json);
    json missed{};

    if (!missing.empty())
        missed = MissingMerge(animelib_json);
    if (!missing.empty())
    {
        spdlog::critical("Some titles were not found in anilib.json, you need to add them manually");
        SaveFile("not_found.json", missing);
    }
    
    //
    //missing not work
    //todo:



    spdlog::info("{}/{} found", main.size(), shiki_size);
    return main;
}

json Parser::MainMerge(const json& anilib, const json& shiki)
{
    json shikilib = json::array();

    if (anilib.empty() || shiki.empty() || !anilib.is_array() || !shiki.is_array())
    {
        spdlog::error("Invalid input in merge method, please check your .json files");
        return shikilib;
    }

    shiki_size = shiki.size();
    anilib_size = anilib.size();

    for (const json& title : shiki)
    {
        std::string target_title = title["target_title"].get<std::string>();

        is_found.store(false);

        std::future<json> fw_result = std::async(std::launch::async, &Parser::SearchDefault, this, true, target_title, std::cref(anilib));
        std::future<json> bw_result = std::async(std::launch::async, &Parser::SearchDefault, this, false, target_title, std::cref(anilib));

        json result_fw = fw_result.get();
        json result_bw = bw_result.get();

        if (!result_fw.empty())
        {
            spdlog::debug("found -> {0}", result_fw["name"].get<std::string>());
            shikilib.push_back(GenerateMergedObject(title, result_fw));
        }
            
        else if (!result_bw.empty())
            shikilib.push_back(GenerateMergedObject(title, result_bw));
        else
            missing.push_back(title);
    }
    return shikilib;
}

json Parser::MissingMerge(const json& anilib)
{
    json shikilib = json::array();
    json missing_temp = json::array();

    for (const json& title : missing)
    {
        is_found.store(false);
        std::string title_ru = title["target_title_ru"].get<std::string>();
        std::string title_en = title["target_title"].get<std::string>();

        std::future<json> fr = std::async(std::launch::async, &Parser::SearchRussian, this, true, title_ru, std::cref(anilib));
        std::future<json> br = std::async(std::launch::async, &Parser::SearchRussian, this, false, title_ru, std::cref(anilib));
        std::future<json> fe = std::async(std::launch::async, &Parser::SearchEnglish, this, true, title_en, std::cref(anilib));
        std::future<json> be = std::async(std::launch::async, &Parser::SearchEnglish, this, false, title_en, std::cref(anilib));
        
        json fw_ru, bw_ru , fw_en, bw_en;
        try 
        {
            fw_ru = fr.get(), bw_ru = br.get(), fw_en = fe.get(), bw_en = be.get();
        }
        catch (const nlohmann::json::type_error& e) 
        {
            spdlog::error("JSON Type error: {}", e.what());
        }
        catch (const nlohmann::json::parse_error& e) 
        {
            spdlog::error("JSON Parse error: {}", e.what());
        }
        catch (const std::exception& e) 
        {
            spdlog::error("General error: {}", e.what());
        }

        if (!fw_ru.empty()) shikilib.push_back(GenerateMergedObject(title, fw_ru));
        else if (!bw_ru.empty()) shikilib.push_back(GenerateMergedObject(title, bw_ru));
        else if (!fw_en.empty()) shikilib.push_back(GenerateMergedObject(title, fw_en));
        else if (!bw_en.empty()) shikilib.push_back(GenerateMergedObject(title, bw_en));
        else missing_temp.push_back(title);
    }

    missing = missing_temp;
    return shikilib;
}

json Parser::SearchDefault(bool from_start, const std::string& target, const json& anilib)
{
    int start = from_start ? 0 : anilib_size - 1;
    int end = from_start ? anilib_size : -1;
    int step = from_start ? 1 : -1;

    for (int i = start; i != end && !is_found.load(); i += step)
    {
        if (anilib[i].contains("name") && !anilib[i]["name"].is_null() && 
            anilib[i]["name"].get<std::string>() == target)
        {
            is_found.store(true);
            return anilib[i];
        }
    }
    return {};
}

json Parser::SearchRussian(bool from_start, const std::string& target, const json& anilib)
{
    int start = from_start ? 0 : anilib_size - 1;
    int end = from_start ? anilib_size : -1;
    int step = from_start ? 1 : -1;

    for (int i = start; i != end && !is_found.load(); i += step)
    {
        if (anilib[i].contains("rus_name") && !anilib[i]["rus_name"].is_null() && 
        anilib[i]["rus_name"].get<std::string>() == target)
    {
            is_found.store(true);
            return anilib[i];
        }
    }
    return {};
}

json Parser::SearchEnglish(bool from_start, const std::string& target, const json& anilib)
{
    int start = from_start ? 0 : anilib_size - 1;
    int end = from_start ? anilib_size : -1;
    int step = from_start ? 1 : -1;

    for (int i = start; i != end && !is_found.load(); i += step)
    {
        if (anilib[i].contains("eng_name") && !anilib[i]["eng_name"].is_null() && 
        anilib[i]["eng_name"].get<std::string>() == target)
        {
            is_found.store(true);
            return anilib[i];
        }
    }
    return {};
}

json Parser::GenerateMergedObject(const json &shiki, const json &anilib)
{
    nlohmann::ordered_json j = nlohmann::ordered_json::object();

    try
    {
        //INFO
        j["info"]["title"]     = (anilib.contains("name") && !anilib["name"].is_null()) ? anilib["name"].get<std::string>() : "Unknown";
        j["info"]["title_en"]   = (anilib.contains("eng_name") && !anilib["eng_name"].is_null()) ? anilib["eng_name"].get<std::string>() : "Unknown";
        j["info"]["title_ru"]   = (anilib.contains("rus_name") && !anilib["rus_name"].is_null()) ? anilib["rus_name"].get<std::string>() : "Unknown";
        j["info"]["score"]      = (shiki.contains("score") && !shiki["score"].is_null()) ? shiki["score"].get<int>() : 0;
        j["info"]["rewatches"]  = (shiki.contains("rewatches") && !shiki["rewatches"].is_null()) ? shiki["rewatches"].get<int>() : 0;
        j["info"]["episodes"]   = (shiki.contains("episodes") && !shiki["episodes"].is_null()) ? shiki["episodes"].get<int>() : 0;
        j["info"]["type"]       = (shiki.contains("target_type") && !shiki["target_type"].is_null()) ? shiki["target_type"].get<std::string>() : "Unknown";
        j["info"]["status"]     = (shiki.contains("status") && !shiki["status"].is_null()) ? shiki["status"].get<std::string>() : "Unknown";
    
        //ID
        j["id"]["shiki"]        = (shiki.contains("target_id") && !shiki["target_id"].is_null()) ? shiki["target_id"].get<int>() : -1;
        j["id"]["anilib"]       = (anilib.contains("id") && !anilib["id"].is_null()) ? anilib["id"].get<int>() : -1;
        j["id"]["slug"]         = (anilib.contains("slug") && !anilib["slug"].is_null()) ? anilib["slug"].get<std::string>() : "no_slug";
        j["id"]["slug_url"]     = (anilib.contains("slug_url") && !anilib["slug_url"].is_null()) ? anilib["slug_url"].get<std::string>() : "no_url";
    
        //cover
        j["cover"]              = (anilib.contains("cover") && !anilib["cover"].is_null()) ? anilib["cover"] : "no_cover";
    
        return j;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Object generating exception: {}", e.what());
        return {};
    }
}

json Parser::ReadFile(const std::string file_name)
{
    json result = {};
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
    catch (const nlohmann::json::parse_error &e)
    {
        spdlog::error("JSON parse error in file {0}: {1} at byte {2}", file_name, e.what(), e.byte);
    }
    return result;
}

void Parser::SaveFile(const std::string file_name, const json &j)
{
    if (file_name.empty() || j.is_null())
    {
        spdlog::error("Cannot save JSON: file name is empty or JSON data is null");
        return;
    }

    try
    {
        std::ofstream file(file_name);
        if (!file.is_open())
            throw std::ios_base::failure("Failed to open file");

        file << j.dump(4);
        spdlog::info("Successfully saved JSON to file: {} (size: {} bytes)", file_name, j.dump().size());
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error while saving file {}: {}", file_name, e.what());
    }
}