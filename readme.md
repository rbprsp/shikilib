# dokidoki

Sync your anime list across platforms. AniLib only for now, Shikimori and the cross-platform mapping resolver are in progress.

# Status

Work in progress, nothing here is stable yet. What works today:

- Pulls your AniLib catalog into a local sqlite db
- Parses a Shikimori list export (no sync back yet)
- Skeleton for cross-platform ID mapping. Cache lookup is wired, the network resolve steps are still stubs.

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

# What's next

The mapping resolver. AniLib and Shikimori don't share IDs :( But shikimori share IDs with MAL and anilist :)

# Why

Tired of synchronizing watched episodes by hand.
