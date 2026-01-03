#include <string>

#include <nlohmann/json.hpp>

class Networker
{
private:
    int page = 1;
    bool fetch_active = true;
    std::string request_url = "https://api.cdnlibs.org/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&page=";
    std::string bookmarks_url = "https://api.cdnlibs.org/api/bookmarks?page=";
    std::string seed = "a30c9942325e3f97316580334e31de3c";  // Seed для запросов
    std::string token = "";
    std::string user_id = "5874032";  // User ID для запросов закладок
    int request_counter = 0;  // Счетчик запросов для AddtoAnimeLib

private:
    std::string PerformRequest(const std::string& url, const std::string& token = "");
    void AddtoAnimeLib(const std::string& body);

public:
    void SetToken(std::string token);
    void SetUserId(std::string user_id);


    nlohmann::json FetchAnimeLib();
    nlohmann::json FetchBookmarks();
    nlohmann::json FilterMissingBookmarks(const nlohmann::json& shikilib_json, const nlohmann::json& existing_bookmarks);

    void AddToAnimeLibFromJson(const nlohmann::json& document);
    void AddToAnimeLibBulk(const std::string& body);
    void AddToAnimeLibFromJsonBulk(const nlohmann::json& document);
};