#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "networker.h"
#include "parser/parser.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out) 
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

/// @brief using inclass members to fetch data
/// @return returns vector of pages
std::vector<nlohmann::json> Networker::FetchAnimeList()
{
    std::vector<nlohmann::json> pages;

    while(this->fetch_active = true)
    {
        std::string responseBody = this->PerformRequest(this->request_url + std::to_string(this->page));
        try 
        {
            auto jsonResponse = nlohmann::json::parse(responseBody);
            if(jsonResponse["data"].empty())
            {
                this->fetch_active = false;
                break;
            }
            else
            {
                spdlog::debug("Fetching page -> {0}, objects on page -> {1}", this->page, jsonResponse["data"].size());
                pages.push_back(jsonResponse);
            }
        } 
        catch (const std::exception& e) 
        {
            spdlog::warn("Unable to parse object on page {0} | {1}", this->page, e.what());
        }

        this->page++;
    }

    return pages;
}

std::string Networker::PerformRequest(const std::string& url)
{
    CURL* curl;
    CURLcode res;
    std::string body;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: meowmeow");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            spdlog::warn("Unable to perform request on page -> {0}", this->page);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
    {
        spdlog::error("Unable to initialize curl");
    }

    return body;
}