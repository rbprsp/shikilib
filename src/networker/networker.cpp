#include <iostream>
#include <thread>
#include <chrono>
#include <set>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <algorithm>

#include "networker.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out) 
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

void Networker::SetToken(std::string token)
{
    this->token = token;
}

void Networker::SetUserId(std::string user_id)
{
    this->user_id = user_id;
}

std::string Networker::PerformRequest(const std::string& url, const std::string& token)
{
    CURL* curl;
    std::string body;

    curl = curl_easy_init();
    if (curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(headers, "Host: api.cdnlibs.org");
        headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0");
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
        headers = curl_slist_append(headers, "Referer: https://anilib.me/");
        headers = curl_slist_append(headers, "Site-Id: 5");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Client-Time-Zone: Asia/Almaty");
        headers = curl_slist_append(headers, "Origin: https://anilib.me");
        headers = curl_slist_append(headers, "DNT: 1");
        headers = curl_slist_append(headers, "Sec-GPC: 1");
        headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
        headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
        headers = curl_slist_append(headers, "Sec-Fetch-Site: cross-site");
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, "TE: trailers");
        if (!token.empty())
        {
            std::string bearer_header = "Authorization: Bearer " + token;
            headers = curl_slist_append(headers, bearer_header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return body;
    }
    else
    {
        spdlog::error("Unable to initialize curl");
        return body;
    }
}

nlohmann::json Networker::FetchAnimeLib()
{
    nlohmann::json result = nlohmann::json::array();

    int error_ctr = 0;
    while (this->fetch_active)
    {
        std::string full_url = this->request_url + std::to_string(this->page) + "&seed=" + this->seed + "&site_id[]=5&sort_by=created_at";
        std::string responseBody = this->PerformRequest(full_url, this->token);
        try 
        {
            auto jsonResponse = nlohmann::json::parse(responseBody);
            if (!jsonResponse.contains("data") || jsonResponse["data"].empty())
            {
                spdlog::debug("No more data to fetch at page {0}", this->page);
                this->fetch_active = false;
                break;
            }
            spdlog::debug("Fetching page -> {0}, objects on page -> {1}", this->page, jsonResponse["data"].size());
            for (const auto& item : jsonResponse["data"]) 
            {
                result.push_back(item);
            }
            this->page++;
            error_ctr = 0;
        } 
        catch (const nlohmann::json::parse_error& e) 
        {
            spdlog::debug("Unable to parse JSON response on page {0}: {1} at byte {2}", 
                         this->page, e.what(), e.byte);
            spdlog::debug("Waiting...");
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            error_ctr++;
        }
        catch (const std::exception& e) 
        {
            spdlog::debug("Unexpected error on page {0}: {1}", this->page, e.what());
            spdlog::debug("Waiting...");
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            error_ctr++;
        }

        if(error_ctr >= 50)
        {
            error_ctr = 0;
            page++;
        }
    }
    spdlog::info("Fetch completed. Total objects fetched: {0}", result.size());

    return result;
}

nlohmann::json Networker::FetchBookmarks()
{
    nlohmann::json result = nlohmann::json::object();
    result["data"] = nlohmann::json::array();
    
    this->page = 1;
    this->fetch_active = true;

    while (this->fetch_active)
    {
        std::string full_url = this->bookmarks_url + std::to_string(this->page) + "&sort_by=name&sort_type=desc&status=0";
        if (!this->user_id.empty())
            full_url += "&user_id=" + this->user_id;

        std::string responseBody = this->PerformRequest(full_url, this->token);
        try
        {
            auto jsonResponse = nlohmann::json::parse(responseBody);

            if(jsonResponse["data"].size() == 0)
                fetch_active = false;

            spdlog::debug("Fetching bookmarks page -> {0}, objects on page -> {1}", this->page, jsonResponse["data"].size());
            for (const auto& item : jsonResponse["data"])
                result["data"].push_back(item);
            this->page++;
        }
        catch(...)
        {
            spdlog::critical("bookmarks fetch error");
        }
    }
    spdlog::info("Bookmarks fetch completed. Total bookmarks fetched: {0}", result["data"].size());

    return result;
}

int MapStatusToAnilibId(const std::string& status)
{
    static const std::unordered_map<std::string, int> status_map = {
        {"completed", 22},    // Просмотрено
        {"watching", 21},     // Смотрю
        {"planned", 23},      // Запланировано
        {"on_hold", 24},      // Отложено
        {"dropped", 25},      // Брошено
        {"rewatching", 26}    // Пересматриваю
    };
    
    auto it = status_map.find(status);
    if (it != status_map.end())
        return it->second;
    spdlog::warn("Unknown status '{}', defaulting to 'planned' (23)", status);
    return 23; 
}

void Networker::AddToAnimeLibFromJson(const nlohmann::json& document) 
{
    if (!document.is_array())
    {
        spdlog::error("Invalid JSON format: expected array");
        return;
    }

    int total_items = document.size();
    int processed = 0;
    int skipped = 0;

    spdlog::info("Starting to add {} items to AnimeLib...", total_items);

    for (const auto& item : document)
    {
        try
        {
            int anilib_id = item["id"]["anilib"].get<int>();
            if (anilib_id <= 0)
            {
                spdlog::warn("Skipping item: invalid anilib id ({})", anilib_id);
                skipped++;
                continue;
            }

            std::string title = "Unknown";
            if (item.contains("info"))
            {
                if (item["info"].contains("title_ru") &&
                    !item["info"]["title_ru"].is_null() &&
                    item["info"]["title_ru"].get<std::string>() != "Unknown")
                {
                    title = item["info"]["title_ru"].get<std::string>();
                }
                else if (item["info"].contains("title_en") &&
                         !item["info"]["title_en"].is_null() &&
                         item["info"]["title_en"].get<std::string>() != "Unknown")
                {
                    title = item["info"]["title_en"].get<std::string>();
                }
                else if (item["info"].contains("title") && !item["info"]["title"].is_null())
                {
                    title = item["info"]["title"].get<std::string>();
                }
            }

            nlohmann::json bookmark_body;
            std::string media_type = item["info"]["type"].get<std::string>();
            std::transform(
                media_type.begin(),
                media_type.end(),
                media_type.begin(),
                [](unsigned char c) { return std::tolower(c); }
            );

            bookmark_body["media_type"] = media_type;
            bookmark_body["media_slug"] = item["id"]["slug"].get<std::string>();

            nlohmann::json bookmark_obj;
            bookmark_obj["status"] = MapStatusToAnilibId(item["info"]["status"].get<std::string>());
            bookmark_body["bookmark"] = bookmark_obj;

            bookmark_body["meta"] = nlohmann::json::object();

            std::string body_str = bookmark_body.dump();

            spdlog::info("Adding [{}/{}]: {} (ID: {}, Status: {})",
                processed + 1,
                total_items,
                title,
                anilib_id,
                bookmark_body["bookmark"]["status"].get<int>());

            spdlog::debug("Request body: {}", body_str);

            AddtoAnimeLib(body_str);
            processed++;
        }
        catch (const nlohmann::json::exception& e)
        {
            spdlog::error("JSON error processing item: {}", e.what());
            skipped++;
        }
        catch (const std::exception& e)
        {
            spdlog::error("Error processing item: {}", e.what());
            skipped++;
        }
    }

    spdlog::info("Total items: {}", total_items);
    spdlog::info("Processed: {}", processed);
}

void Networker::AddtoAnimeLib(const std::string& body) 
{
    CURL* curl;
    CURLcode res;
    std::string response;
    long response_code = 0;
    curl = curl_easy_init();
    if (curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.cdnlibs.org/api/bookmarks");

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Host: api.cdnlibs.org");
        headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:146.0) Gecko/20100101 Firefox/146.0");
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
        headers = curl_slist_append(headers, "Referer: https://anilib.me/");
        headers = curl_slist_append(headers, "Site-Id: 5");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Client-Time-Zone: Asia/Almaty");
        headers = curl_slist_append(headers, "Origin: https://anilib.me");
        headers = curl_slist_append(headers, "DNT: 1");
        headers = curl_slist_append(headers, "Sec-GPC: 1");
        headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
        headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
        headers = curl_slist_append(headers, "Sec-Fetch-Site: cross-site");
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, "TE: trailers");
        if (!token.empty())
        {
            std::string bearer_header = "Authorization: Bearer " + token;
            headers = curl_slist_append(headers, bearer_header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res != CURLE_OK) 
            spdlog::error("Request failed with error: {}. Response code: {}", curl_easy_strerror(res), response_code);
        else 
            spdlog::info("Request completed. Response code: {}. Response: {}", response_code, response);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } 
    else 
        spdlog::error("Failed to initialize CURL. Response code: {}", response_code);

    request_counter++;
    if (request_counter > 0 && request_counter % 15 == 0)
    {
        spdlog::info("Выполнено {} запросов. Ожидание 30 секунд для предотвращения timeout...", request_counter);
        std::this_thread::sleep_for(std::chrono::seconds(15));
        spdlog::info("Продолжение работы после задержки");
    }
}