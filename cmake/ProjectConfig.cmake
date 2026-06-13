
add_library(ProjectConfig INTERFACE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(ProjectConfig INTERFACE
        DEBUG
        LOGGING_ENABLED
        ASSERTS_ENABLED
    )
else()
    target_compile_definitions(ProjectConfig INTERFACE
        NDEBUG
    )
endif()

if (PROJECT_PLATFORM STREQUAL "LINUX")
    target_compile_definitions(ProjectConfig INTERFACE
        PLATFORM_LINUX=1
    )
endif()

target_compile_features(ProjectConfig INTERFACE cxx_std_20)
