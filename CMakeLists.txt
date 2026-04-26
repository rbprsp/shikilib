cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME "dokidoki")
project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES C CXX
    DESCRIPTION "Sync between multiple anime watch & track platforms"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(GIT_SSL_NO_VERIFY ON CACHE BOOL "" FORCE)
set(FETCHCONTENT_QUIET OFF)

# ---- GLAZE -----------------------
find_package(glaze QUIET)
if(NOT glaze_FOUND)
  fetchcontent_declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG main
    )
  fetchcontent_makeavailable(glaze)
endif()

# ---- CPR -------------------------
fetchcontent_declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG master
)
fetchcontent_makeavailable(cpr)

find_package(CURL REQUIRED)

# ---- SPDLOG ----------------------
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
  fetchcontent_declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x
    )
  fetchcontent_makeavailable(spdlog)
endif()

# ---- SQLITE3 ---------------------
find_package(SQLite3 REQUIRED)

# ---- SQLITE-ORM ------------------
fetchcontent_declare(
    sqlite_orm
    GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
    GIT_TAG master
)
fetchcontent_makeavailable(sqlite_orm)

# ---- TOML11 -----------------------
fetchcontent_declare(
    toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG v4.4.0
)
fetchcontent_makeavailable(toml11)

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/config/config.cpp"
    "src/networker/networker.cpp"
    "src/utils/utils.cpp"
    "src/anilib/auth/auth.cpp"
    "src/anilib/catalog/catalog.cpp"
    "src/db/db.cpp"
    "src/shiki/list/shiki_list.cpp"
    "src/mapping/resolver.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog_header_only
    cpr::cpr
    glaze::glaze
    SQLite3::SQLite3
    sqlite_orm
    toml11::toml11
)
