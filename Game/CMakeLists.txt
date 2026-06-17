project(Game)

set(GENERATED_OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated)

add_executable(Game
    ${GENERATED_OUTPUT_DIR}/TextId.cpp
    src/Game.cpp
    src/Localization.cpp
    src/Main.cpp
)

file(COPY ${CMAKE_BINARY_DIR}/assets
     DESTINATION ${CMAKE_BINARY_DIR}/Game)

target_include_directories(Game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GENERATED_OUTPUT_DIR}
)

target_link_libraries(Game PRIVATE
    Common
)
