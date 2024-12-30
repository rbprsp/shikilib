#include <string>
#include <vector>

class Networker
{
private: 
/*private members*/
    int page = 1;
    int max_pages = 370;        // current last page (useless)
    bool fetch_active = true;

    //url without page
    std::string request_url = "https://api2.mangalib.me/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5&page=";

private: 
/*private forward declarations:*/
    std::string PerformRequest(const std::string& url);

public:
/*public forward declarations*/

    std::vector<nlohmann::json> FetchAnimeList();

    int AddToPlanned(int shikimori_id);
    int AddToCompleted(int shikimori_id);
    int AddToWatching(int shikimori_id);
    int AddToDropped(int shikimori_id);
    
};