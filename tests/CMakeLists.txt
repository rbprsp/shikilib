add_executable(doki_tests
    "mapping.cpp"
    "${CMAKE_SOURCE_DIR}/src/mapping/resolver.cpp"
    "${CMAKE_SOURCE_DIR}/src/db/db.cpp"
)

target_include_directories(doki_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(doki_tests PRIVATE
    Catch2::Catch2WithMain
    spdlog::spdlog_header_only
    glaze::glaze
    sqlite_orm
    SQLite3::SQLite3
)

include(Catch)
catch_discover_tests(doki_tests)
