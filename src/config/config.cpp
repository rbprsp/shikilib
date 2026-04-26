#include "config/config.h"
#include "toml11/find.hpp"

#include <spdlog/spdlog.h>

#include <toml.hpp>

static void StripBearerPrefix(std::string &token)
{
    const std::string prefix = "Bearer ";
    if (token.starts_with(prefix))
    {
        token = token.substr(prefix.size());
        spdlog::debug("Stripped 'Bearer ' prefix from token");
    }
}

Config ConfigLoader::Load(const std::string& path)
{
    Config cfg;

    try
    {
        const auto data = toml::parse(path);

        if (data.contains("anilib"))
        {
            const auto& anilib = toml::find(data, "anilib");
            if (anilib.contains("api_token"))
            {
                cfg.anilib.api_token = toml::find<std::string>(anilib, "api_token");
                StripBearerPrefix(cfg.anilib.api_token);
            }

            if (anilib.contains("host_url"))
            {
                cfg.anilib.host_url = toml::find<std::string>(anilib, "host_url");
            }
        }

        if (data.contains("shikimori"))
        {
            const auto& shiki = toml::find(data, "shikimori");
            cfg.shikimori.client_id = toml::find_or<std::string>(shiki, "client_id", "");
            cfg.shikimori.client_secret = toml::find_or<std::string>(shiki, "client_secret", "");
            cfg.shikimori.redirect_uri = toml::find_or<std::string>(shiki, "redirect_uri", "");
            cfg.shikimori.user_agent = toml::find_or<std::string>(shiki, "user_agent", "");
        }

        if (data.contains("database"))
        {
            const auto& db = toml::find(data, "database");
            cfg.database.path = toml::find_or<std::string>(db, "path", "dokidoki.db");
        }

        if (data.contains("logging"))
        {
            const auto& log = toml::find(data, "logging");
            cfg.logging.level = toml::find_or<std::string>(log, "level", "info");
        }

        spdlog::info("Config loaded from {}", path);
    }
    catch (const std::exception& e)
    {
        spdlog::warn("Failed to load config from {}: {}", path, e.what());
        return LoadDefault();
    }

    return cfg;
}

Config ConfigLoader::LoadDefault()
{
    Config cfg;
    spdlog::info("Using default configuration");
    return cfg;
}
