project(AssetBuilder)

add_executable(AssetBuilder
    src/Application.cpp
    src/Localization.cpp
    src/Main.cpp
)

target_include_directories(AssetBuilder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AssetBuilder PRIVATE
    Common
    spdlog::spdlog_header_only
    CLI11::CLI11
    simdjson
)
