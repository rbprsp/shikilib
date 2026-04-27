# dokidoki

Sync your anime list across platforms. AniLib only for now, Shikimori and the cross-platform mapping resolver are in progress.

# Status

Work in progress. What works today:

* List synchronization shikimori -> anilib 


# Build

You need CMake 3.21+, a C++20 compiler, libcurl, and sqlite3 on the system. Everything else is fetched at configure time.

```sh
cmake -B build
cmake --build build
```

# Configure

Copy `config.toml.example` to `config.toml` and fill in your tokens.
You can read about anilib token and hosturl [here](docs/anilibtoken.md).

```toml
[anilib]
api_token = "..."
host_url = "" #anilib host url
user_id = "..."

[shikimori]
client_id = "..."
client_secret = "..."

[logging]
level = "info"
```

# Run

```sh
./build/dokidoki              # reads ./config.toml
./build/dokidoki my.toml      # custom path
```

The first run dumps every page of your AniLib catalog into `pages/*.json` and writes `anilib.db`.

# TODO

- [x] The mapping resolver.
- [ ] cli commands
- [ ] flutter ui

# Why

Tired of synchronizing watched episodes by hand.
