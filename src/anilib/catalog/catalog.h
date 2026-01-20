#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <vector>

struct Cover
{
    std::optional<std::string> filename;
    std::string thumbnail;
    std::string default_;
    std::string md;

    struct glaze
    {
        using T = Cover;

        static constexpr auto value =
            glz::object("filename", &T::filename, "thumbnail", &T::thumbnail, "default", &T::default_, "md", &T::md);
    };
};

struct AgeRestriction
{
    int id{};
    std::string label{};
};

struct Type
{
    int id{};
    std::string label{};
};

struct Rating
{
    std::string average;
    std::string averageFormated;
    int votes{};
    std::string votesFormated;
    int user{};
};

struct Status
{
    int id{};
    std::string label{};
};

struct UserBookmarkMeta
{
    bool comment{};
    std::optional<int> rewatches;
    std::optional<int> item_number;

    struct glaze
    {
        using T = UserBookmarkMeta;

        static constexpr auto value =
            glz::object("comment", &T::comment, "rewatches", &T::rewatches, "item_number", &T::item_number);
    };
};

struct UserBookmark
{
    int id{};
    std::string type;
    int media_id{};

    std::optional<int> item_id;
    std::optional<int> progress;

    int status{};

    std::string created_at;
    std::string updated_at;

    UserBookmarkMeta meta;

    struct glaze
    {
        using T = UserBookmark;

        static constexpr auto value = glz::object(
            "id", &T::id, "type", &T::type, "media_id", &T::media_id, "item_id", &T::item_id, "progress", &T::progress,
            "status", &T::status, "created_at", &T::created_at, "updated_at", &T::updated_at, "meta", &T::meta);
    };
};

struct AnimeItem
{
    int id{};
    std::string name;
    std::string rus_name;
    std::optional<std::string> eng_name;
    std::string model;
    std::string slug;
    std::string slug_url;

    Cover cover;
    AgeRestriction ageRestriction;
    int site{};
    Type type;
    Rating rating;

    std::optional<UserBookmark> userBookmark;
    std::vector<glz::generic> content_marking;
    Status status;

    std::string releaseDateString;
    std::optional<std::string> shiki_rate;

    struct glaze
    {
        using T = AnimeItem;
        static constexpr auto value =
            glz::object("id", &T::id, "name", &T::name, "rus_name", &T::rus_name, "eng_name", &T::eng_name, "model",
                        &T::model, "slug", &T::slug, "slug_url", &T::slug_url, "cover", &T::cover, "ageRestriction",
                        &T::ageRestriction, "site", &T::site, "type", &T::type, "rating", &T::rating, "userBookmark",
                        &T::userBookmark, "content_marking", &T::content_marking, "status", &T::status,
                        "releaseDateString", &T::releaseDateString, "shiki_rate", &T::shiki_rate);
    };
};

struct PaginationLinks
{
    std::string first;
    std::optional<std::string> last;
    std::optional<std::string> prev;
    std::optional<std::string> next;

    struct glaze
    {
        using T = PaginationLinks;

        static constexpr auto value =
            glz::object("first", &T::first, "last", &T::last, "prev", &T::prev, "next", &T::next);
    };
};

struct PaginationMeta
{
    int current_page{};
    std::optional<int> from;
    std::string path;
    int per_page{};
    std::optional<int> to;
    std::string seed;

    struct glaze
    {
        using T = PaginationMeta;
        static constexpr auto value = glz::object("current_page", &T::current_page, "from", &T::from, "path", &T::path,
                                                  "per_page", &T::per_page, "to", &T::to, "seed", &T::seed);
    };
};

struct AnilibResponse
{
    std::vector<AnimeItem> data;
    PaginationLinks links;
    PaginationMeta meta;

    struct glaze
    {
        using T = AnilibResponse;

        static constexpr auto value = glz::object("data", &T::data, "links", &T::links, "meta", &T::meta);
    };
};

class Catalog
{
private:
    std::string token = "";
    std::string seed = "";
    std::string base_url =
        "https://api.cdnlibs.org/api/anime?fields[]=rate&fields[]=rate_avg&fields[]=userBookmark&site_id[]=5";

public:
    Catalog(std::string token);

    AnilibResponse ParsePage(int page);
};