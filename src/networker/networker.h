#include <string>

class Networker
{
private: 
    int page = 1;
    bool fetch_active = true;
    std::string request_url = "https://api2.mangalib.me/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5&page=";

private:
    std::string PerformCookieRequest(const std::string& url, const std::string& cookie);
    std::string PerformRequest      (const std::string& url);

public:
    nlohmann::json FetchAnimeLib();

    static void AddtoAnimeLib();
};