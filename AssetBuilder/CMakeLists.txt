project(AssetBuilder)

set(LOC_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/strings)
set(LOC_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Generated/strings)

add_executable(AssetBuilder
    ${LOC_OUTPUT_DIR}/TextId.cpp
    src/Application.cpp
    src/Localization.cpp
    src/Main.cpp
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res
     DESTINATION ${CMAKE_BINARY_DIR}/AssetBuilder)

target_compile_definitions(AssetBuilder PUBLIC
    LOC_OUTPUT_DIR="${LOC_OUTPUT_DIR}"
)

target_include_directories(AssetBuilder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LOC_OUTPUT_DIR}
)

target_link_libraries(AssetBuilder PRIVATE
    Common
    spdlog::spdlog_header_only
    CLI11::CLI11
    simdjson
)
