project(AssetBuilder)

add_executable(AssetBuilder
    src/Utils/Log.cpp
    src/Main.cpp
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res
     DESTINATION ${CMAKE_BINARY_DIR}/AssetBuilder)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(AssetBuilder PUBLIC
        DEBUG
        LOGGING_ENABLED
        ASSERTS_ENABLED
    )
else()
    target_compile_definitions(AssetBuilder PUBLIC
        NDEBUG
    )
endif()

if (PROJECT_PLATFORM STREQUAL "LINUX")
    target_compile_definitions(AssetBuilder PUBLIC
        PLATFORM_LINUX=1
    )
endif()

target_include_directories(AssetBuilder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(AssetBuilder PRIVATE cxx_std_20)

target_link_libraries(AssetBuilder PRIVATE
    spdlog::spdlog_header_only
    CLI11::CLI11
)
