#include "RenderableChunk.h"
#include "Environment/Environment.h"

#include <glm/glm/gtx/vector_angle.hpp>

RenderableChunk::RenderableChunk(unsigned int LODindex, int positionInWorldX, int positionInWorldZ, ChunkManager& manager) :
	Chunk(positionInWorldX, positionInWorldZ),
	mLODIndex(LODindex),
	mLODInterval(0.5 * (pow(2, LODindex + 1))),
	mManagerForThisChunk(manager)
{
	using namespace Framework;
	
	mSeamCorrectedHeights.resize((mMaxNumSquares + 1) * (mMaxNumSquares + 1));

	unsigned int gridSize = mManagerForThisChunk.mChunkGridSize;
	mElementInParentArray = (((mXPositionInWorld / mMaxNumSquares) + floor(gridSize / 2)) * gridSize) + ((mZPositionInWorld / mMaxNumSquares) + floor(gridSize / 2));
	
	mTerrainModel = std::make_unique<Model3D>();

	manager.mResourceSet->addMesh("chunkMesh" + std::to_string(manager.mChunks.size()), GL_TRIANGLES, manager.mChunkTexture, manager.mChunkShader, glm::translate(glm::mat4(1), glm::vec3(positionInWorldX, 0, positionInWorldZ)));
	mMesh = manager.mResourceSet->getResource<Graphics::Mesh>("chunkMesh" + std::to_string(manager.mChunks.size()));
	mTerrainModel->addMesh(manager.mResourceSet->getResource<Graphics::Mesh>("chunkMesh" + std::to_string(manager.mChunks.size())));
	//FINE UP TO HERE

	setUpGLData();
	initMesh();
}

void RenderableChunk::changeLOD(int offset) {
	if (mLODIndex + offset >= 0 && mLODIndex + offset < mManagerForThisChunk.getNumLODs())
		mMesh->addIndexBuffer(mManagerForThisChunk.mLODIndexBuffers[mLODIndex + offset]);
}

void RenderableChunk::checkCulling(glm::vec3 cameraFront, glm::vec3 cameraPosition, float horizontalFOV) {
	glm::vec3 playerToChunkLowest = cameraPosition - glm::vec3(mXPositionInWorld, 0, mZPositionInWorld);
	float horizontalAngle = 180 - glm::degrees(glm::angle(glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z)), glm::normalize(glm::vec3(playerToChunkLowest.x, 0, playerToChunkLowest.z))));

	if (horizontalAngle > horizontalFOV * 0.5f || glm::length(playerToChunkLowest) > (mManagerForThisChunk.mChunkGridSize * 0.5f + 4) * mMaxNumSquares)
		mHidden = true;
	else
		mHidden = false;
}

void RenderableChunk::updateNormalsArray(bool innerNormalsDone) {
	if(!innerNormalsDone)
		updateInnerNormals();

	updateEdgeNormals();
}

void RenderableChunk::updateHeightsArrayForSeams() {
	mSeamCorrectedHeights = mHeights;

	//Seam patching up
	int gridSize = mManagerForThisChunk.mChunkGridSize;
	int currentChunkIndex = (((mXPositionInWorld / mMaxNumSquares) + floor(gridSize / 2)) * gridSize) + ((mZPositionInWorld / mMaxNumSquares) + floor(gridSize / 2));
	int neighbourChunkIndex = 0, neighbourLODInterval = 0, startElement = 0;
	float x0 = 0.0f, x1 = 0.0f;

	if (onLODBorderX()) {
		if (mXPositionInWorld < 0) { //Left edge is affected
			neighbourChunkIndex = currentChunkIndex - gridSize;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = (mMaxNumSquares + 1) * mMaxNumSquares;

			for (int i = mLODInterval; i < mMaxNumSquares + 1; i += neighbourLODInterval) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i - mLODInterval)];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i + mLODInterval)];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}
		}
		else if (mXPositionInWorld > 0) { //Right edge is affected
			neighbourChunkIndex = currentChunkIndex + gridSize;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = 0;

			for (int i = mLODInterval; i < mMaxNumSquares + 1; i += neighbourLODInterval) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i - mLODInterval)];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i + mLODInterval)];

				mSeamCorrectedHeights[mMaxNumSquares * (mMaxNumSquares + 1) + i] = (x0 + x1) / 2;
			}
		}
		else { //Both edges must be affected
			   //Left side is patched up
			neighbourChunkIndex = currentChunkIndex - gridSize;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = (mMaxNumSquares + 1) * mMaxNumSquares;

			for (int i = mLODInterval; i < mMaxNumSquares + 1; i += neighbourLODInterval) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i - mLODInterval)];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i + mLODInterval)];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}

			//Right side is patched up
			neighbourChunkIndex = currentChunkIndex + gridSize;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = 0;

			for (int i = mLODInterval; i < mMaxNumSquares + 1; i += neighbourLODInterval) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i - mLODInterval)];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[startElement + (i + mLODInterval)];

				mSeamCorrectedHeights[mMaxNumSquares * (mMaxNumSquares + 1) + i] = (x0 + x1) / 2;
			}
		}
	}
	if (onLODBorderZ()) {
		if (mZPositionInWorld < 0) { //Bottom edge is affected
			neighbourChunkIndex = currentChunkIndex - 1;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = mMaxNumSquares;

			for (int i = mLODInterval * (mMaxNumSquares + 1); i < mMaxNumSquares * (mMaxNumSquares + 1); i += neighbourLODInterval * (mMaxNumSquares + 1)) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i - mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i + mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}
		}
		else if (mZPositionInWorld > 0) { //Top edge is affected
			neighbourChunkIndex = currentChunkIndex + 1;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = 0;

			for (int i = mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares; i < pow(mMaxNumSquares + 1, 2); i += neighbourLODInterval * (mMaxNumSquares + 1)) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i - mLODInterval * (mMaxNumSquares + 1) - mMaxNumSquares];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i + mLODInterval * (mMaxNumSquares + 1) - mMaxNumSquares];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}
		}
		else { //Both edges must be affected
			   //Bottom edge is patched up
			neighbourChunkIndex = currentChunkIndex - 1;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = mMaxNumSquares;

			for (int i = mLODInterval * (mMaxNumSquares + 1); i < mMaxNumSquares * (mMaxNumSquares + 1); i += neighbourLODInterval * (mMaxNumSquares + 1)) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i - mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i + mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}

			//Top edge is patched up
			neighbourChunkIndex = currentChunkIndex + 1;
			neighbourLODInterval = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mLODInterval;
			startElement = 0;

			for (int i = mLODInterval * (mMaxNumSquares + 1) + mMaxNumSquares; i < pow(mMaxNumSquares + 1, 2); i += neighbourLODInterval * (mMaxNumSquares + 1)) {
				x0 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i - mLODInterval * (mMaxNumSquares + 1) - mMaxNumSquares];
				x1 = mManagerForThisChunk.mChunks[neighbourChunkIndex]->mHeights[i + mLODInterval * (mMaxNumSquares + 1) - mMaxNumSquares];

				mSeamCorrectedHeights[i] = (x0 + x1) / 2;
			}
		}
	}
}

void RenderableChunk::render(Framework::Graphics::Renderer& renderer) {
	mTerrainModel->sendRenderCommands(renderer);
}

void RenderableChunk::initMesh() {
	using namespace Framework::Graphics;

	//Sets the buffers with empty data (all 0s), so that update can be called afterwards
	mHeightBuffer->updateData(mHeights);
	mNormalsBuffer->updateData(mNormals);
}

void RenderableChunk::setUpGLData() {
	using namespace Framework::Graphics;

	//Tie the XZ position buffer to the mesh
	mMesh->addBuffer(/*mManagerForThisChunk.mXZChunkPositionData*/mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkXZBuffer"));
	
	//Tie the Y position buffer to the mesh
	mManagerForThisChunk.mResourceSet->addVertexBuffer(GL_DYNAMIC_DRAW, VertexFormat(1, 1, GL_FLOAT, false), mHeights, "chunkHeights" + std::to_string(mManagerForThisChunk.mChunks.size()));
	mMesh->addBuffer(mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkHeights" + std::to_string(mManagerForThisChunk.mChunks.size())));
	mHeightBuffer = mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkHeights" + std::to_string(mManagerForThisChunk.mChunks.size()));

	//Tie the texture coordinate buffer to the mesh
	mMesh->addBuffer(mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkTextureCoords")/*mManagerForThisChunk.mChunkTextureCoords*/);

	//Tie the normals buffer to the mesh
	mManagerForThisChunk.mResourceSet->addVertexBuffer(GL_DYNAMIC_DRAW, VertexFormat(3, 3, GL_FLOAT, false), mNormals, "chunkNormals" + std::to_string(mManagerForThisChunk.mChunks.size()));
	mMesh->addBuffer(mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkNormals" + std::to_string(mManagerForThisChunk.mChunks.size())));
	mNormalsBuffer = mManagerForThisChunk.mResourceSet->getResource<VertexBuffer>("chunkNormals" + std::to_string(mManagerForThisChunk.mChunks.size()));

	//Tie the index buffer to the mesh
	mMesh->addIndexBuffer(mManagerForThisChunk.mLODIndexBuffers[mLODIndex]);
}

bool RenderableChunk::onLODBorderX() {
	int gridSize = mManagerForThisChunk.mChunkGridSize;
	int currentChunkIndex = (((mXPositionInWorld / mMaxNumSquares) + floor(gridSize / 2)) * gridSize) + ((mZPositionInWorld / mMaxNumSquares) + floor(gridSize / 2));

	//As long as there are chunks to the left and the right of this one
	if ((currentChunkIndex >= gridSize) && (currentChunkIndex < gridSize * (gridSize - 1))) {
		int LODOfChunkToLeft = mManagerForThisChunk.mChunks[currentChunkIndex - gridSize]->mLODInterval;
		int LODOfChunkToRight = mManagerForThisChunk.mChunks[currentChunkIndex + gridSize]->mLODInterval;

		//If the LOD of the chunk being examined is different to either LOD of the chunks next door to it on the x axis, we must be on a boundary
		//If this is a single outer node chunk on the circumference...
		if (LODOfChunkToLeft == LODOfChunkToRight && mLODInterval != LODOfChunkToLeft)
			return true;
		//If the chunk is to the right, check the right chunk's LOD
		else if ((mXPositionInWorld > 0 && LODOfChunkToRight != mLODInterval) || (mXPositionInWorld < 0 && LODOfChunkToLeft != mLODInterval))
			return true;
		else
			return false;
	}
	else
		return false;
}

bool RenderableChunk::onLODBorderZ() {
	int gridSize = mManagerForThisChunk.mChunkGridSize;
	int currentChunkIndex = (((mXPositionInWorld / mMaxNumSquares) + floor(gridSize / 2)) * gridSize) + ((mZPositionInWorld / mMaxNumSquares) + floor(gridSize / 2));

	//As long as there are chunks in front and behind this one                                                                                                                                                                                                                                                                                                                                            e chunks to the left and the right
	if ((currentChunkIndex % gridSize != 0) && ((currentChunkIndex + 1) % gridSize != 0)) {
		int LODOfChunkInFront = mManagerForThisChunk.mChunks[currentChunkIndex + 1]->mLODInterval;
		int LODOfChunkBehind = mManagerForThisChunk.mChunks[currentChunkIndex - 1]->mLODInterval;

		//If the LOD of the chunk being examined is different to either LOD of the chunks next door to it on the x axis, we must be on a boundary
		//If this is a single outer node chunk on the circumference...
		if (LODOfChunkInFront == LODOfChunkBehind && mLODInterval != LODOfChunkInFront)
			return true;
		//If depending on the chunk having a positive or negative z coord, the LOD boundaries are also found
		else if ((mZPositionInWorld > 0 && LODOfChunkInFront != mLODInterval) || (mZPositionInWorld < 0 && LODOfChunkBehind != mLODInterval))
			return true;
		else
			return false;
	}
	else
		return false;
}

void RenderableChunk::updateEdgeNormals() {
	//If this chunk is not on the edge of the grid
	if ((abs(mXPositionInWorld / mMaxNumSquares) != (mManagerForThisChunk.mChunkGridSize - 1) / 2) && (abs(mZPositionInWorld / mMaxNumSquares) != (mManagerForThisChunk.mChunkGridSize - 1) / 2)) {
		updateLeftEdgeNormals();
		updateRightEdgeNormals();
		updateTopEdgeNormals();
		updateBottomEdgeNormals();
	}
}

void RenderableChunk::updateLeftEdgeNormals() {
	glm::vec3 normal(0.0f);
	double lowerLeft, left, lower, upper, upperRight, right, thisHeight;
	int gridSize = mManagerForThisChunk.mChunkGridSize;

	for (int z = 0; z < mMaxNumSquares + 1; z++) {
		thisHeight = mHeights[z];

		if (z == 0) {
			lowerLeft = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize - 1]->mHeights[(mMaxNumSquares + 1) * mMaxNumSquares - 2];
			lower = mManagerForThisChunk.mChunks[mElementInParentArray - 1]->mHeights[mMaxNumSquares - 1];
			left = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize]->mHeights[(mMaxNumSquares + 1) * (mMaxNumSquares - 1)];
			upper = mHeights[1];
			upperRight = mHeights[mMaxNumSquares + 2];
			right = mHeights[mMaxNumSquares + 1];
		}
		else if (z == mMaxNumSquares) {
			lowerLeft = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize]->mHeights[(mMaxNumSquares + 1) * (mMaxNumSquares - 1) + z - 1];
			lower = mHeights[z - 1];
			left = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize]->mHeights[(mMaxNumSquares + 1) * (mMaxNumSquares - 1) + z];
			upper = mManagerForThisChunk.mChunks[mElementInParentArray + 1]->mHeights[1];
			upperRight = mManagerForThisChunk.mChunks[mElementInParentArray + 1]->mHeights[mMaxNumSquares + 2];
			right = mHeights[mMaxNumSquares + 1 + z];
		}
		else {
			lowerLeft = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize]->mHeights[(mMaxNumSquares + 1) * (mMaxNumSquares - 1) + z - 1];
			lower = mHeights[z - 1];
			left = mManagerForThisChunk.mChunks[mElementInParentArray - gridSize]->mHeights[(mMaxNumSquares + 1) * (mMaxNumSquares - 1) + z];
			upper = mHeights[z + 1];
			upperRight = mHeights[mMaxNumSquares + 2 + z];
			right = mHeights[mMaxNumSquares + 1 + z];
		}
		normal += glm::cross(glm::vec3(0, lowerLeft, -1), glm::vec3(1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, thisHeight, 1), glm::vec3(-1, lowerLeft, 0)) +
			glm::cross(glm::vec3(0, upper, 1), glm::vec3(-1, left, 0)) +
			glm::cross(glm::vec3(0, thisHeight, -1), glm::vec3(1, upperRight, 0)) +
			glm::cross(glm::vec3(0, upperRight, 1), glm::vec3(-1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, lower, -1), glm::vec3(1, right, 0));

		normal = glm::normalize(normal);

		mNormals[z * 3] = normal.x;
		mNormals[z * 3 + 1] = normal.y;
		mNormals[z * 3 + 2] = normal.z;
	}
}

void RenderableChunk::updateRightEdgeNormals() {
	glm::vec3 normal(0.0f);
	double lowerLeft, left, lower, upper, upperRight, right, thisHeight;
	int gridSize = mManagerForThisChunk.mChunkGridSize;

	//Right edge
	for (int z = mMaxNumSquares * (mMaxNumSquares + 1); z < (mMaxNumSquares + 1) * (mMaxNumSquares + 1); z++) {
		thisHeight = mHeights[z];

		if (z == mMaxNumSquares * (mMaxNumSquares + 1)) {
			lowerLeft = mManagerForThisChunk.mChunks[mElementInParentArray - 1]->mHeights[z - 2];
			lower = mManagerForThisChunk.mChunks[mElementInParentArray - 1]->mHeights[z + (mMaxNumSquares - 1)];
			left = mHeights[z - (mMaxNumSquares + 1)];
			upper = mHeights[z + 1];
			upperRight = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize]->mHeights[mMaxNumSquares + 2];
			right = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize]->mHeights[mMaxNumSquares + 1];
		}
		else if (z == (mMaxNumSquares + 1) * (mMaxNumSquares + 1) - 1) {
			lowerLeft = mHeights[mMaxNumSquares * (mMaxNumSquares + 1) - 2];
			lower = mHeights[z - 1];
			left = mHeights[z - mMaxNumSquares - 1];
			upper = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize + 1]->mHeights[1];
			upperRight = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize + 1]->mHeights[mMaxNumSquares + 2];
			right = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize + 1]->mHeights[mMaxNumSquares + 1];
		}
		else {
			lowerLeft = mHeights[z - mMaxNumSquares - 2];
			lower = mHeights[z - 1];
			left = mHeights[z - mMaxNumSquares - 1];
			upper = mHeights[z + 1];
			upperRight = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize]->mHeights[z - (mMaxNumSquares * mMaxNumSquares - 2)];
			right = mManagerForThisChunk.mChunks[mElementInParentArray + gridSize]->mHeights[z - (mMaxNumSquares * mMaxNumSquares - 1)];
		}

		normal += glm::cross(glm::vec3(0, lowerLeft, -1), glm::vec3(1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, thisHeight, 1), glm::vec3(-1, lowerLeft, 0)) +
			glm::cross(glm::vec3(0, upper, 1), glm::vec3(-1, left, 0)) +
			glm::cross(glm::vec3(0, thisHeight, -1), glm::vec3(1, upperRight, 0)) +
			glm::cross(glm::vec3(0, upperRight, 1), glm::vec3(-1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, lower, -1), glm::vec3(1, right, 0));

		normal = glm::normalize(normal);

		mNormals[z * 3] = normal.x;
		mNormals[z * 3 + 1] = normal.y;
		mNormals[z * 3 + 2] = normal.z;
	}
}

void RenderableChunk::updateTopEdgeNormals() {
	glm::vec3 normal(0.0f);
	double lowerLeft, left, lower, upper, upperRight, right, thisHeight;
	int gridSize = mManagerForThisChunk.mChunkGridSize;

	//Top edge
	for (int x = mMaxNumSquares * 2 + 1; x < (mMaxNumSquares + 1) * (mMaxNumSquares + 1) - 1; x += mMaxNumSquares + 1) {
		thisHeight = mHeights[x];

		lowerLeft = mHeights[x - mMaxNumSquares - 2];
		lower = mHeights[x - 1];
		left = mHeights[x - mMaxNumSquares - 1];
		upper = mManagerForThisChunk.mChunks[mElementInParentArray + 1]->mHeights[x - (mMaxNumSquares - 1)];
		upperRight = mManagerForThisChunk.mChunks[mElementInParentArray + 1]->mHeights[x + 2];
		right = mHeights[x + mMaxNumSquares + 1];

		normal += glm::cross(glm::vec3(0, lowerLeft, -1), glm::vec3(1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, thisHeight, 1), glm::vec3(-1, lowerLeft, 0)) +
			glm::cross(glm::vec3(0, upper, 1), glm::vec3(-1, left, 0)) +
			glm::cross(glm::vec3(0, thisHeight, -1), glm::vec3(1, upperRight, 0)) +
			glm::cross(glm::vec3(0, upperRight, 1), glm::vec3(-1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, lower, -1), glm::vec3(1, right, 0));

		normal = glm::normalize(normal);

		mNormals[x * 3] = normal.x;
		mNormals[x * 3 + 1] = normal.y;
		mNormals[x * 3 + 2] = normal.z;
	}
}

void RenderableChunk::updateBottomEdgeNormals() {
	glm::vec3 normal(0.0f);
	double lowerLeft, left, lower, upper, upperRight, right, thisHeight;
	int gridSize = mManagerForThisChunk.mChunkGridSize;

	//Top edge
	for (int x = mMaxNumSquares + 1; x < mMaxNumSquares * (mMaxNumSquares + 1); x += mMaxNumSquares + 1) {
		thisHeight = mHeights[x];

		lowerLeft = mManagerForThisChunk.mChunks[mElementInParentArray - 1]->mHeights[x - 2];
		lower = mManagerForThisChunk.mChunks[mElementInParentArray - 1]->mHeights[x + (mMaxNumSquares - 1)];
		left = mHeights[x - mMaxNumSquares - 1];
		upper = mHeights[x + 1];
		upperRight = mHeights[x + mMaxNumSquares + 2];
		right = mHeights[x + mMaxNumSquares + 1];

		normal += glm::cross(glm::vec3(0, lowerLeft, -1), glm::vec3(1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, thisHeight, 1), glm::vec3(-1, lowerLeft, 0)) +
			glm::cross(glm::vec3(0, upper, 1), glm::vec3(-1, left, 0)) +
			glm::cross(glm::vec3(0, thisHeight, -1), glm::vec3(1, upperRight, 0)) +
			glm::cross(glm::vec3(0, upperRight, 1), glm::vec3(-1, thisHeight, 0)) +
			glm::cross(glm::vec3(0, lower, -1), glm::vec3(1, right, 0));

		normal = glm::normalize(normal);

		mNormals[x * 3] = normal.x;
		mNormals[x * 3 + 1] = normal.y;
		mNormals[x * 3 + 2] = normal.z;
	}
}

void RenderableChunk::updateGLData() {
	mHeightBuffer->updateData(mSeamCorrectedHeights);
	mNormalsBuffer->updateData(mNormals);
}