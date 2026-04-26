#ifndef ANILIB_MODELS_H
#define ANILIB_MODELS_H

#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <vector>

struct Cover
{
    std::optional<std::string> filename;
    std::string thumbnail;
    std::string default_img;
    std::string md;

    struct glaze
    {
        using T = Cover;

        static constexpr auto value =
            glz::object("filename", &T::filename, "thumbnail", &T::thumbnail, "default", &T::default_img, "md", &T::md);
    };
};

struct AgeRestriction
{
    int id{};
    std::string label;
};

struct EmptyType
{
    struct glaze
    {
        using T = EmptyType;
        static constexpr auto value = glz::array();
    };
};

struct Type
{
    int id{};
    std::string label;
};

struct Rating
{
    std::string average;
    std::string average_formated;
    std::optional<int> votes;
    std::optional<std::string> votes_formated;
    std::optional<int> user;

    struct glaze
    {
        using T = Rating;
        static constexpr auto value =
            glz::object("average", &T::average, "averageFormated", &T::average_formated, "votes", &T::votes,
                        "votesFormated", &T::votes_formated, "user", &T::user);
    };
};

struct Status
{
    int id{};
    std::string label;
};

struct UserBookmarkMeta
{
    bool comment{};
    std::optional<int> rewatches;
    std::optional<int> item_number;
    std::optional<std::string> player;
    std::optional<int> team;
    std::optional<int> translation_type;

    struct glaze
    {
        using T = UserBookmarkMeta;

        static constexpr auto value =
            glz::object("comment", &T::comment, "rewatches", &T::rewatches, "item_number", &T::item_number, "player",
                        &T::player, "team", &T::team, "translation_type", &T::translation_type);
    };
};

struct UserBookmark
{
    int id{};
    std::string type;
    int media_id{};

    std::optional<int> item_id;
    std::optional<std::string> progress;

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
    std::optional<std::string> rus_name;
    std::optional<std::string> eng_name;
    std::string model;
    std::string slug;
    std::string slug_url;

    Cover cover;
    AgeRestriction age_restriction;
    int site{};
    std::variant<Type, EmptyType> type;
    Rating rating;

    std::optional<UserBookmark> user_bookmark;
    std::vector<std::string> content_marking;
    Status status;

    std::string release_date_string;
    std::optional<std::string> shiki_rate;
    std::optional<std::string> anilist_id;

    struct glaze
    {
        using T = AnimeItem;
        static constexpr auto value = glz::object(
            "id", &T::id, "name", &T::name, "rus_name", &T::rus_name, "eng_name", &T::eng_name, "model", &T::model,
            "slug", &T::slug, "slug_url", &T::slug_url, "cover", &T::cover, "ageRestriction", &T::age_restriction,
            "site", &T::site, "type", &T::type, "rating", &T::rating, "userBookmark", &T::user_bookmark,
            "content_marking", &T::content_marking, "status", &T::status, "releaseDateString", &T::release_date_string,
            "shiki_rate", &T::shiki_rate, "anilist_id", &T::anilist_id);
    };
};

struct Links
{
    std::string first;
    std::optional<std::string> last;
    std::optional<std::string> prev;
    std::optional<std::string> next;

    struct glaze
    {
        using T = Links;

        static constexpr auto value =
            glz::object("first", &T::first, "last", &T::last, "prev", &T::prev, "next", &T::next);
    };
};

struct Meta
{
    int current_page{};
    std::optional<int> from;
    std::string path;
    int per_page{};
    std::optional<int> to;
    std::string seed;

    struct glaze
    {
        using T = Meta;
        static constexpr auto value = glz::object("current_page", &T::current_page, "from", &T::from, "path", &T::path,
                                                  "per_page", &T::per_page, "to", &T::to, "seed", &T::seed);
    };
};

struct AnilibCatalog
{
    std::vector<AnimeItem> data;
    Links links;
    Meta meta;

    struct glaze
    {
        using T = AnilibCatalog;

        static constexpr auto value = glz::object("data", &T::data, "links", &T::links, "meta", &T::meta);
    };
};

struct Anime
{
    int id = 0;

    std::string name;
    std::string rus_name;
    std::string eng_name;
    std::string type;
    std::string slug;
    std::string slug_url;
    std::optional<int> shiki_id;
    std::optional<std::string> shiki_name;
    std::optional<std::string> shiki_status;
    std::optional<int> shiki_score;
};

#endif
