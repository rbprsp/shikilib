//STL
#include <iostream>
#include <fstream>
#include <thread>
#include <future>

//EXTERNAL
#include <spdlog/spdlog.h>

//LOCAL
#include "parser.h"

#ifdef __linux__
    //TODO:

#elif _WIN32
    #include <windows.h>
    #include <commdlg.h>

#else
    //TODO:
#endif

json Parser::Merge(const json& animelib_json, const json& shikimori_json)
{
    json main = MainMerge(animelib_json, shikimori_json);
    json missed {};

    if(!missing.empty())
        missed = MissingMerge(std::cref(animelib_json));
    if(!missing.empty())
    {
        spdlog::critical("Some titles was not found in anilib.json, you need to add them manualy");
        SaveFile("not_found.json", missing);
    }
    if(!missed.empty())
        main.push_back(missed);

    return main;
}

json Parser::MainMerge(const json &anilib, const json &shiki)
{
    json shikilib = json::array();

    //simple checks, I guess
    if(anilib.empty() || shiki.empty() || !anilib.is_array() || !shiki.is_array())
    {
        spdlog::error("Invalid input in merge method, please check your .json files");
        return shikilib;
    }

    //total entries here
    shiki_size  =  shiki.size();
    anilib_size = anilib.size();

    for(const json title : shiki)
    {
        is_found.store(false);

        //no checks here, cuz shiki.json must be auto generated
        std::string target_title = title["target_title"].get<std::string>();

        json fw_result{}, bw_result{};
        std::thread  forward(&Parser::SearchDefault, this, true,  target_title, std::cref(anilib), std::ref(fw_result));
        std::thread backward(&Parser::SearchDefault, this, false, target_title, std::cref(anilib), std::ref(bw_result));

        while(!is_found.load())
            std::this_thread::yield();

        if(forward.joinable())
            forward.join();
        if(backward.joinable())
            backward.join();

        //check if title is missing
        if (!fw_result.empty())
            shikilib.push_back(GenerateMergedObject(title, fw_result));
        else if (!bw_result.empty())
            shikilib.push_back(GenerateMergedObject(title, bw_result));
        else
            missing.push_back(title);
    }
    return shikilib;
}

json Parser::MissingMerge(const json &anilib)
{
    json shikilib     {};
    json missing_temp {};
    json fw_ru, fw_en, bw_ru, bw_en;

    for(const json title : missing)
    {
        std::string title_ru = title["target_title_ru"].get<std::string>();
        std::string title_en = title["target_title"].get<std::string>();

        //ru
        std::thread fr(&Parser::SearchRussian, this, true,  title_ru, std::cref(anilib), std::ref(fw_ru));
        std::thread br(&Parser::SearchRussian, this, false, title_ru, std::cref(anilib), std::ref(bw_ru));

        //en
        std::thread fe(&Parser::SearchRussian, this,  true, title_ru, std::cref(anilib), std::ref(bw_ru));
        std::thread be(&Parser::SearchRussian, this, false, title_ru, std::cref(anilib), std::ref(bw_ru));

        while(!is_found.load())
            std::this_thread::yield();

        if(fr.joinable()) fr.join();
        if(br.joinable()) br.join();
        if(fe.joinable()) be.join();
        if(be.joinable()) be.join();

        if     (!fw_ru.empty()) shikilib.push_back(GenerateMergedObject(title, fw_ru));
        else if(!bw_ru.empty()) shikilib.push_back(GenerateMergedObject(title, bw_ru));
        else if(!fw_en.empty()) shikilib.push_back(GenerateMergedObject(title, fw_en));
        else if(!bw_en.empty()) shikilib.push_back(GenerateMergedObject(title, bw_en));
        else missing_temp.push_back(title);
    }
    missing = missing_temp;
    return shikilib;
}

void Parser::SearchDefault(bool from_start, const std::string &target, const json &anilib, json &result)
{
    int start, end, step;
    if(from_start)
    {
        start = 0;
        end   = anilib_size - 1;
        step  = 1; 
    }
    else
    {
        start = anilib_size - 1;
        end   = -1;
        step  = -1; 
    }

    for(int i = start; i != end && !is_found.load(); i += step)
    {
        auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        spdlog::debug("ID: 0x{:X}, searching for -> {}", thread_id, target);
        if(anilib[i]["name"].get<std::string>() == target)
        {
            is_found.store(true);
            result = anilib[i];
        }
    }
    is_found.store(false);
}

void Parser::SearchRussian(bool from_start, const std::string &target, const json &anilib, json &result)
{
    int start, end, step;
    if(from_start)
    {
        start = 0;
        end   = anilib_size - 1;
        step  = 1; 
    }
    else
    {
        start = anilib_size - 1;
        end   = -1;
        step  = -1; 
    }

    for(int i = start; i != end && !is_found.load(); i += step)
    {
        auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        spdlog::debug("ID: 0x{:X}, searching for -> {}", thread_id, target);
        if(anilib[i]["rus_name"].get<std::string>() == target)
        {
            is_found.store(true);
            result = anilib[i];
        }
    }
    is_found.store(false);
}
void Parser::SearchEnglish(bool from_start, const std::string &target, const json &anilib, json &result)
{
    int start, end, step;
    if(from_start)
    {
        start = 0;
        end   = anilib_size - 1;
        step  = 1; 
    }
    else
    {
        start = anilib_size - 1;
        end   = -1;
        step  = -1; 
    }

    for(int i = start; i != end && !is_found.load(); i += step)
    {
        auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        spdlog::debug("ID: 0x{:X}, searching for -> {}", thread_id, target);
        if(anilib[i]["eng_name"].get<std::string>() == target)
        {
            is_found.store(true);
            result = anilib[i];
        }
    }
    is_found.store(false);
}

json Parser::GenerateMergedObject(const json &shiki, const json &anilib)
{
    nlohmann::ordered_json j = nlohmann::ordered_json::object();

    try
    {
        //INFO
        j["info"]["title"]        = anilib["name"].get<std::string>();
        j["info"]["title_en"]     = anilib["eng_name"].get<std::string>();
        j["info"]["title_ru"]     = anilib["rus_name"].get<std::string>();
        j["info"]["score"]        =  shiki["score"].get<int>();
        j["info"]["rewatches"]    =  shiki["rewatches"].get<int>();
        j["info"]["episodes"]     =  shiki["episodes"].get<int>();
        j["info"]["type"]         =  shiki["target_type"].get<std::string>();
        j["info"]["status"]       =  shiki["status"].get<std::string>();

        //ID
        j["id"]["shiki"]          =  shiki["target_id"].get<int>();
        j["id"]["anilib"]         = anilib["id"].get<int>();
        j["id"]["slug"]           = anilib["slug"].get<std::string>(); 
        j["id"]["slug-url"]       = anilib["slug-url"].get<std::string>();

        //cover
        j["cover"]                = anilib["cover"];

        return j;
    }
    catch(const std::exception& e)
    {
        spdlog::error("Object generating exception: {0}", e.what());
        return {};
    }
}

json Parser::ReadFile(const std::string file_name)
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

void Parser::SaveFile(const std::string file_name, const json& j)
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