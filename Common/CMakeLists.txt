project(Common)

add_library(Common STATIC
    src/Utils/Log.cpp
)

target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Common PUBLIC
    ProjectConfig
    spdlog::spdlog_header_only
)
