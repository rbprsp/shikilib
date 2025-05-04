#include <iostream>
#include <curl/curl.h>
#include <spdlog/spdlog.h>

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

        headers = curl_slist_append(headers, "Host: api.cdnlibs.org");
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
    //TODO:
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

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.cdnlibs.org/api/bookmarks");

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Host: api.cdnlibs.org");
        headers = curl_slist_append(headers, "User-Agent: shikilib");
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