#ifndef RENDERABLECHUNK_H
#define RENDERABLECHUNK_H
#pragma once

#include "../src/Objects/Model3D.h"
#include "Chunk.h"
	
class Environment;

class RenderableChunk : public Chunk {
	friend class ChunkManager;

private:
	std::unique_ptr<Framework::Model3D> mTerrainModel;
	Framework::Graphics::Mesh* mMesh = nullptr;
	Framework::Graphics::VertexBuffer* mHeightBuffer = nullptr;
	Framework::Graphics::VertexBuffer* mNormalsBuffer = nullptr;

	unsigned int 
		mLODIndex = 0,
		mLODInterval = 0;

	int mElementInParentArray = 0;
	bool mHidden = false;

	std::vector<float> mSeamCorrectedHeights;

	ChunkManager& mManagerForThisChunk;

public:
	RenderableChunk(unsigned int LODindex, int positionInWorldX, int positionInWorldZ, ChunkManager& manager);
	~RenderableChunk() = default;

	void changeLOD(int offset);
	void checkCulling(glm::vec3 cameraFront, glm::vec3 cameraPosition, float horizontalFOV);
	void updateNormalsArray(bool innerNormalsDone);
	void updateHeightsArrayForSeams();
	void render(Framework::Graphics::Renderer& renderer);
	
	inline int getLOD() { return mLODInterval; }

private:
	void initMesh();
	void setUpGLData();
	bool onLODBorderX();
	bool onLODBorderZ();
	void updateEdgeNormals();
	void updateLeftEdgeNormals();
	void updateRightEdgeNormals();
	void updateTopEdgeNormals();
	void updateBottomEdgeNormals();
	void updateGLData();

};

#endif
