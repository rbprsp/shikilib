#include <iostream>
#include <conio.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "networker.h"
#include "parser/parser.h"
#include "utils/utils.h"

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

std::string Networker::PerformRequest(const std::string& url, const std::string& token)
{
    CURL* curl;
    CURLcode res;
    std::string body;

    curl = curl_easy_init();
    if (curl) 
    {
        std::string full_token = "Authorization: " + token;
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
        headers = curl_slist_append(headers, full_token.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        res = curl_easy_perform(curl);
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

    bool auth_request = false;

    int error_ctr = 0;
    while (this->fetch_active)
    {
        std::string responseBody = this->PerformRequest(this->request_url + std::to_string(this->page), this->token);
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

void Networker::AddToAnimeLibFromJson(const nlohmann::json& document) 
{
    if (!document.is_array()) 
    {
        spdlog::error("Document is not a JSON array");
        return;
    }

    for (const auto& item : document) 
    {
        if (!item.is_object()) 
        {
            spdlog::warn("Skipping invalid element: not an object");
            continue;
        }

        std::string slug_url;
        int status_anilib = 0;
        std::string media_type;

        // Extract slug_url (будет использовано как media_slug)
        if (item.contains("info") && item["info"].contains("slug_url") && item["info"]["slug_url"].is_string()) 
        {
            slug_url = item["info"]["slug_url"].get<std::string>();
        } 
        else 
        {
            spdlog::warn("Missing or invalid slug_url in object");
            continue;
        }

        // Extract status_anilib
        if (item.contains("bookmark") && item["bookmark"].contains("status_anilib") && item["bookmark"]["status_anilib"].is_number_integer()) 
        {
            status_anilib = item["bookmark"]["status_anilib"].get<int>();
        } 
        else 
        {
            spdlog::warn("Missing or invalid status_anilib in object with slug_url: {}", slug_url);
            continue;
        }

        // Extract model (будет использовано как media_type)
        if (item.contains("info") && item["info"].contains("model") && item["info"]["model"].is_string()) 
        {
            media_type = item["info"]["model"].get<std::string>();
        } 
        else 
        {
            media_type = "anime"; // По умолчанию "anime", если model отсутствует
            spdlog::debug("No model found for slug_url: {}, using default media_type: anime", slug_url);
        }

        // Form request body in the exact format
        nlohmann::json request_body = {
            {"media_type", media_type},
            {"media_slug", slug_url},
            {"bookmark", {{"status", status_anilib}}},
            {"meta", nlohmann::json::object()}
        };

        std::string body_str = request_body.dump();

        spdlog::info("Sending request for media_slug: {}, status: {}, media_type: {}", slug_url, status_anilib, media_type);

        AddtoAnimeLib(body_str);
    }

    spdlog::info("Processing of all objects completed");
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
        std::string full_token = "Authorization: " + token;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api2.mangalib.me/api/bookmarks");

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Host: api2.mangalib.me");
        headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:136.0) Gecko/20100101 Firefox/136.0");
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
        headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(body.length())).c_str());
        headers = curl_slist_append(headers, "Referer: https://anilib.me/");
        headers = curl_slist_append(headers, "Site-Id: 5");
        headers = curl_slist_append(headers, "Origin: https://anilib.me");
        headers = curl_slist_append(headers, "DNT: 1");
        headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
        headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
        headers = curl_slist_append(headers, "Sec-Fetch-Site: cross-site");
        headers = curl_slist_append(headers, full_token.c_str());
        headers = curl_slist_append(headers, "Connection: keep-alive");
        headers = curl_slist_append(headers, "Priority: u=0");
        headers = curl_slist_append(headers, "TE: trailers");

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
}