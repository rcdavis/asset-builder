project(Game)

set(LOC_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res/strings)
set(LOC_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Generated/strings)

add_executable(Game
    ${LOC_OUTPUT_DIR}/TextId.cpp
    src/Main.cpp
)

target_compile_definitions(Game PUBLIC
    LOC_OUTPUT_DIR="${LOC_OUTPUT_DIR}"
)

target_include_directories(Game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LOC_OUTPUT_DIR}
)

target_link_libraries(Game PRIVATE
    Common
)
