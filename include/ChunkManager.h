#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#pragma once

#include "Framework.h"
#include "Chunk/RenderableChunk.h"

typedef std::vector<std::unique_ptr<RenderableChunk>> RenderableChunkVector;
typedef std::vector<std::unique_ptr<Chunk>> ChunkVector;

class ChunkManager {
	friend class RenderableChunk;

private:
	Framework::ResourceSet* mResourceSet = nullptr;
	
	int mChunkGridSize = 0;

	RenderableChunkVector mChunks;
	ChunkVector mEdgeBufferChunks;
	
	Framework::Graphics::Texture2D* mChunkTexture = nullptr;
	Framework::Graphics::Shader* mChunkShader = nullptr;
	Framework::Graphics::VertexBuffer* mXZChunkPositionData = nullptr;
	Framework::Graphics::VertexBuffer* mChunkTextureCoords = nullptr;
	std::vector<Framework::Graphics::IndexBuffer*> mLODIndexBuffers;

	//The environment this ChunkManager belongs to
	Environment* mEnvironment = nullptr;

	std::vector<int> mLODBounds = { 13, 7, 3 }; //13, 7, 3
	unsigned short mNumLODs = 4;

	enum edge { LEFT, TOP, RIGHT, BOTTOM };

public:
	ChunkManager(unsigned int chunkGridSize, Environment* e, Framework::ResourceSet* resourceSet);
	~ChunkManager() = default;

	void cullChunks(glm::vec3& cameraFront, glm::vec3& cameraPosition, Framework::PerspectiveCamera* camera);
	void updateChunksAlongX(int direction);
	void updateChunksAlongZ(int direction);
	void renderChunks(Framework::Graphics::Renderer& renderer);
	
	inline unsigned short getNumLODs() { return mNumLODs; }
	inline int getChunkGridSize() { return mChunkGridSize; }
	inline Environment* getEnvironment() { return mEnvironment; }
	inline int getHighestLODChunkNumSquares() { return Chunk::mMaxNumSquares; }
	inline RenderableChunk& getChunk(int index) { return *mChunks[index]; }
	inline RenderableChunkVector& getMainChunks() { return mChunks; }
	inline RenderableChunk& getPlayerChunk() { return *mChunks[trunc(pow(mChunkGridSize, 2) / 2)]; }
	inline Framework::Graphics::Texture2D* getChunkTexture() const { return mChunkTexture; }
	inline Framework::Graphics::Shader* getChunkShader() { return mChunkShader; }
	inline std::vector<int>& getLODBounds() { return mLODBounds; }

private:
	void loadChunks();
	void loadCentralChunks();
	void loadBufferChunks();
	void changeBufferChunkPositions(int xDirection, int zDirection);
	void pushChunkBufferIntoMain(unsigned int buffer);
	void updateBufferChunks(unsigned int edge);
	void instantiateSharedResources();
	void loadSharedResources();
	void initShaderUniforms();

};

#endif
