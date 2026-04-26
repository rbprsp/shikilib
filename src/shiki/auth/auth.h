#ifndef SHIKI_AUTH_H
#define SHIKI_AUTH_H

#include <cpr/cpr.h>
#include <string>

struct OAuthToken
{
    std::string access_token;
    std::string refresh_token;
    int expires_in{};
};

class ShikimoriClient
{
private:
    std::string client_id;
    std::string client_secret;
    std::string redirect_uri;
    std::string user_agent;

    OAuthToken token{};

    cpr::Response PostToken(const cpr::Payload &payload);

public:
    ShikimoriClient(const std::string &client_id, const std::string &client_secret, const std::string &redirect_uri,
                    const std::string &user_agent);

    std::string GetAuthorizationUrl(const std::string &scope) const;

    bool ExchangeCodeForToken(const std::string &authorization_code);
    bool RefreshAccessToken();

    cpr::Response GetWhoAmI();
    cpr::Response AuthorizedGet(const std::string &endpoint);

    const OAuthToken &GetToken() const;
};

#endif
