cmake_minimum_required(3.17)
project(procedural-terrain)

add_executable(procedural-terrain
    src/Chunk.cpp
    src/ChunkManager.cpp
    src/Environment.cpp
    src/main.cpp
    src/Player.cpp
    src/RenderableChunk.cpp
    src/Sun.cpp
    src/Terrain.cpp
    src/TerrainGame.cpp
)

target_include_directories(procedural-terrain PUBLIC_HEADER
    include
)