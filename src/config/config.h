#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config
{
    struct Anilib
    {
        std::string api_token;
        std::string host_url;
        std::string user_id;
    } anilib;

    struct Shikimori
    {
        std::string client_id;
        std::string client_secret;
        std::string redirect_uri;
        std::string user_agent;
    } shikimori;

    struct Database
    {
        std::string path = "dokidoki.db";
    } database;

    struct Logging
    {
        std::string level = "info";
    } logging;
};

class ConfigLoader
{
public:
    static Config Load(const std::string& path);
    static Config LoadDefault();
};

#endif
