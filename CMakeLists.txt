cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME "dokidoki")
project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Sync between multiple anime watch & track platforms"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---- GLAZE -----------------------
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

# ---- CPR -------------------------
FetchContent_Declare(
    cpr 
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cpr)

# ---- SPDLOG ----------------------
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
)
FetchContent_MakeAvailable(spdlog)

# ---- SQLITE3 ---------------------
FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake
    GIT_TAG master
)
FetchContent_MakeAvailable(sqlite3)

# ---- SQLITE-ORM ------------------
FetchContent_Declare(
    sqlite_orm
    GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(sqlite_orm)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/networker/networker.cpp
    src/anilib/utils/utils.cpp
    src/anilib/catalog/catalog.h # templates huh
    src/anilib/catalog/catalog.cpp
    src/db/db.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog_header_only
    cpr::cpr
    glaze::glaze
    SQLite::SQLite3
    sqlite_orm 
)