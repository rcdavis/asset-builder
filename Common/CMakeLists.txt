project(Common)

add_library(Common STATIC
    src/Utils/Log.cpp
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Common PUBLIC
        DEBUG
        LOGGING_ENABLED
        ASSERTS_ENABLED
    )
else()
    target_compile_definitions(Common PUBLIC
        NDEBUG
    )
endif()

if (PROJECT_PLATFORM STREQUAL "LINUX")
    target_compile_definitions(Common PUBLIC
        PLATFORM_LINUX=1
    )
endif()

target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(Common PUBLIC cxx_std_20)

target_link_libraries(Common PUBLIC
    spdlog::spdlog_header_only
)
