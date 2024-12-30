cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "shikilib")
set(CMAKE_CXX_STANDARD 20)

if(WIN32)

    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")

    
    string(APPEND CMAKE_C_FLAGS_DEBUG " /MTd")
    string(APPEND CMAKE_C_FLAGS_RELEASE " /MT")

    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /MTd")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /MT")

elseif(UNIX)
    #TODO:
endif()

project(${PROJECT_NAME}
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Sync between animelib and shikimori"
)

find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/parser/parser.cpp
    src/networker/networker.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    CURL::libcurl
    fmt::fmt
    unofficial::sqlite3::sqlite3
)

include_directories("src/")
