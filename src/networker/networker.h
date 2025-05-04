#include <string>

#include <nlohmann/json.hpp>

class Networker
{
private: 
    int page = 1;
    bool fetch_active = true;
    std::string request_url = "https://api.cdnlibs.org/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5&page=";
    std::string token = "";

private:
    std::string PerformRequest(const std::string& url, const std::string& token = "");
    void AddtoAnimeLib(const std::string& body);

public:
    void SetToken(std::string token);


    nlohmann::json FetchAnimeLib();

    
    void AddToAnimeLibFromJson(const nlohmann::json& document);
};