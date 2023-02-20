cmake_minimum_required(VERSION 3.17)
project(procedural-terrain)

set(SOURCES
    src/Chunk.cpp
    src/ChunkManager.cpp
    src/Environment.cpp
    src/Player.cpp
    src/RenderableChunk.cpp
    src/Sun.cpp
    src/Terrain.cpp
    src/TerrainGame.cpp
)


add_library(pt-lib STATIC ${SOURCES})
target_include_directories(pt-lib PRIVATE include)

add_executable(pt-app src/main.cpp)
target_link_libraries(pt-app pt-lib)

set(PT_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)