#include <iostream>
#include <conio.h>
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
            spdlog::warn("Unable to perform request on page -> {0}", this->page);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
        spdlog::error("Unable to initialize curl");

    return body;
}

std::string Networker::PerformCookieRequest(const std::string& url, const std::string& cookie)
{
    CURL* curl;
    CURLcode res;
    std::string body;

    curl = curl_easy_init();
    if (curl) 
    {
        std::string full_cookie = "Authorization: " + cookie;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(headers, "Host: api2.mangalib.me");
        headers = curl_slist_append(headers, "User-Agent: shikilib");
        headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
        headers = curl_slist_append(headers, "DNT: 1");
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
        headers = curl_slist_append(headers, "Sec-Fetch-Dest: document");
        headers = curl_slist_append(headers, "Sec-Fetch-Mode: navigate");
        headers = curl_slist_append(headers, "Sec-Fetch-Site: none");
        headers = curl_slist_append(headers, "Sec-Fetch-User: ?1");
        headers = curl_slist_append(headers, "TE: trailers");
        headers = curl_slist_append(headers, full_cookie.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            spdlog::warn("Unable to perform request on page -> {0}", this->page);
        else
            spdlog::debug("Page -> {0} [check]", page);

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

//TODO:
nlohmann::json Networker::FetchAnimeLib()
{
    nlohmann::json result = nlohmann::json::array();

    int error_ctr = 0;
    while (this->fetch_active)
    {
        std::string responseBody = this->PerformCookieRequest(this->request_url + std::to_string(this->page), "");
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

void Networker::AddtoAnimeLib()
{
   //todo
}