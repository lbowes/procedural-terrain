#ifndef TERRAIN_H
#define TERRAIN_H
#pragma once

#include "ChunkManager.h"

class Player;

class Terrain {
private:
	unsigned int mChunkGridSize = 0;
	ChunkManager mChunkManager;

public:
	Terrain(unsigned int chunkGridSize, Environment* e, Framework::ResourceSet* r);
	~Terrain() = default;

	inline ChunkManager& getChunkManager() { return mChunkManager; }

	void update(Player& player);
	void render(Framework::Graphics::Renderer& renderer);
	double getGroundHeight(glm::vec2& positionInChunk);

};

#endif