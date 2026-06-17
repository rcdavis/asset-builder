project(Game)

set(GENERATED_OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated)

add_executable(Game
    ${GENERATED_OUTPUT_DIR}/TextId.cpp
    src/Game.cpp
    src/Localization.cpp
    src/Main.cpp
)

file(GLOB_RECURSE ASSET_FILES
    "${CMAKE_SOURCE_DIR}/assets/*"
)

add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/.assets_stamp"

    COMMAND $<TARGET_FILE:AssetBuilder> localization
            -i "${CMAKE_SOURCE_DIR}/assets"
            -o "${CMAKE_BINARY_DIR}/Game/assets"
            -g "${GENERATED_OUTPUT_DIR}"

    COMMAND ${CMAKE_COMMAND} -E touch
            "${CMAKE_BINARY_DIR}/.assets_stamp"

    DEPENDS
        AssetBuilder
        ${ASSET_FILES}

    COMMENT "Building game assets"
)

add_custom_target(BuildAssets
    DEPENDS "${CMAKE_BINARY_DIR}/.assets_stamp"
)

add_dependencies(Game BuildAssets)

target_include_directories(Game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GENERATED_OUTPUT_DIR}
)

target_link_libraries(Game PRIVATE
    Common
)
