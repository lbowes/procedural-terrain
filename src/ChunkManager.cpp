#include "ChunkManager.h"
#include "Environment.h"
#include "Utils/ConsoleLogger.h"
#include "Graphics/Texture/Texture2D.h"

ChunkManager::ChunkManager(unsigned int chunkGridSize, Environment* e, Framework::ResourceSet* resourceSet) :
	mChunkGridSize(chunkGridSize % 2 == 0 ? chunkGridSize + 1 : chunkGridSize),
	mEnvironment(e),
	mResourceSet(resourceSet)
{
	instantiateSharedResources();
	loadSharedResources();

	loadChunks();
}

void ChunkManager::cullChunks(glm::vec3& cameraFront, glm::vec3& cameraPosition, Framework::PerspectiveCamera* camera) {
	for (const auto& chunk : mChunks)
		chunk->checkCulling(cameraFront, cameraPosition, camera->getHorizontalFOV());
}

void ChunkManager::updateChunksAlongX(int direction) {
	switch (direction) {
	case 1:
		for (int x = 0; x < mChunkGridSize - 1; x++) {
			for (int z = 0; z < mChunkGridSize; z++) {
				mChunks[x * mChunkGridSize + z]->mHeights = mChunks[x * mChunkGridSize + z + mChunkGridSize]->mHeights;
				mChunks[x * mChunkGridSize + z]->mNormals = mChunks[x * mChunkGridSize + z + mChunkGridSize]->mNormals;
			}
		}
		changeBufferChunkPositions(1, 0);
		updateBufferChunks(RIGHT);
		pushChunkBufferIntoMain(RIGHT);
		for (const auto& chunk : mChunks) {
			chunk->updateHeightsArrayForSeams();
			chunk->updateNormalsArray(false);
			chunk->updateGLData();
		}
		break;

	case -1:
		for (int x = mChunkGridSize - 1; x > 0; x--) {
			for (int z = 0; z < mChunkGridSize; z++) {
				mChunks[x * mChunkGridSize + z]->mHeights = mChunks[x * mChunkGridSize + z - mChunkGridSize]->mHeights;
				mChunks[x * mChunkGridSize + z]->mNormals = mChunks[x * mChunkGridSize + z - mChunkGridSize]->mNormals;
			}
		}
		changeBufferChunkPositions(-1, 0);
		updateBufferChunks(LEFT);
		pushChunkBufferIntoMain(LEFT);
		for (const auto& chunk : mChunks) {
			chunk->updateHeightsArrayForSeams();
			chunk->updateNormalsArray(false);
			chunk->updateGLData();
		}
		break;

	default:
		Framework::Utils::ConsoleLogger::log(Framework::Utils::ConsoleLogger::LogType::ERROR_HALT, "Tried to update chunks along the X axis with parameter '" + std::to_string(direction) + "'. Only valid arguments are '1' and '-1'");
		break;
	}


}

void ChunkManager::updateChunksAlongZ(int direction) {
	switch (direction) {
	case 1:
		for (int x = 0; x < mChunkGridSize; x++) {
			for (int z = 0; z < mChunkGridSize - 1; z++) {
				mChunks[x * mChunkGridSize + z]->mHeights = mChunks[x * mChunkGridSize + z + 1]->mHeights;
				mChunks[x * mChunkGridSize + z]->mNormals = mChunks[x * mChunkGridSize + z + 1]->mNormals;
			}
		}
		changeBufferChunkPositions(0, 1);
		updateBufferChunks(TOP);
		pushChunkBufferIntoMain(TOP);
		for (const auto& chunk : mChunks) {
			chunk->updateHeightsArrayForSeams();
			chunk->updateNormalsArray(false);
			chunk->updateGLData();
		}
		break;

	case -1:
		for (int x = 0; x < mChunkGridSize; x++) {
			for (int z = mChunkGridSize - 1; z > 0; z--) {
				mChunks[x * mChunkGridSize + z]->mHeights = mChunks[x * mChunkGridSize + z - 1]->mHeights;
				mChunks[x * mChunkGridSize + z]->mNormals = mChunks[x * mChunkGridSize + z - 1]->mNormals;
			}
		}
		changeBufferChunkPositions(0, -1);
		updateBufferChunks(BOTTOM);
		pushChunkBufferIntoMain(BOTTOM);
		for (const auto& chunk : mChunks) {
			chunk->updateHeightsArrayForSeams();
			chunk->updateNormalsArray(false);
			chunk->updateGLData();
		}
		break;

	default:
		Framework::Utils::ConsoleLogger::log(Framework::Utils::ConsoleLogger::LogType::ERROR_HALT, "Tried to update chunks along the Z axis with parameter '" + std::to_string(direction) + "'. Only valid arguments are '1' and '-1'");
		break;
	}
}

void ChunkManager::renderChunks(Framework::Graphics::Renderer& renderer) {
	for (const auto& chunk : mChunks) {
		if (!chunk->mHidden)
			chunk->render(renderer);
	}
}

void ChunkManager::loadChunks() {
	loadCentralChunks();
	loadBufferChunks();
}

void ChunkManager::loadCentralChunks() {
	int
		LOD = 0,
		a = Chunk::mMaxNumSquares;

	unsigned int
		absX = 0,
		absZ = 0;

	for (int x = -mChunkGridSize / 2; x <= mChunkGridSize / 2; x++) {
		for (int z = -mChunkGridSize / 2; z <= mChunkGridSize / 2; z++) {

			absX = abs(x);
			absZ = abs(z);

			if ((absX * absX) + (absZ * absZ) > pow(mLODBounds[0], 2))
				LOD = 3;
			else if ((absX * absX) + (absZ * absZ) > pow(mLODBounds[1], 2))
				LOD = 2;
			else if ((absX * absX) + (absZ * absZ) > pow(mLODBounds[2], 2))
				LOD = 1;
			else
				LOD = 0;

			mChunks.push_back(std::make_unique<RenderableChunk>(LOD, x * a, z * a, *this));

			//deciding if the chunk is within the circle of visible chunks
			if ((absX * absX) + (absZ * absZ) > pow((mChunkGridSize + 1) / 2, 2))
				mChunks.back()->mHidden = true;
		}
	}

	//Updates the heights and normals for the first time (so the terrain is not flat on loading)
	for (const auto& chunk : mChunks)
		chunk->updateHeightsArray();

	for (const auto& chunk : mChunks) {
		chunk->updateHeightsArrayForSeams();
		chunk->updateNormalsArray(false);
		chunk->updateGLData();
	}
}

void ChunkManager::loadBufferChunks() {
	//Left edge loaded from bottom to top
	for (int z = -mChunkGridSize / 2; z <= mChunkGridSize / 2; z++) {
		mEdgeBufferChunks.push_back(std::make_unique<Chunk>((-mChunkGridSize / 2) * Chunk::mMaxNumSquares, z * Chunk::mMaxNumSquares));
		mEdgeBufferChunks.back()->updateHeightsArray();
		mEdgeBufferChunks.back()->updateInnerNormals();
	}
	//Top edge loaded from left to right
	for (int x = -mChunkGridSize / 2; x <= mChunkGridSize / 2; x++) {
		mEdgeBufferChunks.push_back(std::make_unique<Chunk>(x  * Chunk::mMaxNumSquares, (mChunkGridSize / 2) * Chunk::mMaxNumSquares));
		mEdgeBufferChunks.back()->updateHeightsArray();
		mEdgeBufferChunks.back()->updateInnerNormals();
	}
	//Right edge loaded from top to bottom
	for (int z = mChunkGridSize / 2; z >= -mChunkGridSize / 2; z--) {
		mEdgeBufferChunks.push_back(std::make_unique<Chunk>((mChunkGridSize / 2) * Chunk::mMaxNumSquares, z * Chunk::mMaxNumSquares));
		mEdgeBufferChunks.back()->updateHeightsArray();
		mEdgeBufferChunks.back()->updateInnerNormals();
	}
	//Bottom edge loaded from right to left
	for (int x = mChunkGridSize / 2; x >= -mChunkGridSize / 2; x--) {
		mEdgeBufferChunks.push_back(std::make_unique<Chunk>(x * Chunk::mMaxNumSquares, (-mChunkGridSize / 2) * Chunk::mMaxNumSquares));
		mEdgeBufferChunks.back()->updateHeightsArray();
		mEdgeBufferChunks.back()->updateInnerNormals();
	}
}

void ChunkManager::changeBufferChunkPositions(int xDirection, int zDirection) {
	for (const auto& chunk : mEdgeBufferChunks)
		chunk->changePosition(xDirection, zDirection);
}

void ChunkManager::pushChunkBufferIntoMain(unsigned int buffer) {
	switch (buffer) {
	case edge::LEFT:
		//Copy chunk data from left edge buffer into left of mChunks
		for (unsigned int z = 0; z < mChunkGridSize; z++)
			mChunks[z]->recieveNewData(mEdgeBufferChunks[z].get());
	break;

	case edge::TOP:
		//Copy chunk data from top edge buffer into left of mChunks
		for (unsigned int x = mChunkGridSize - 1; x < pow(mChunkGridSize, 2); x += mChunkGridSize)
			mChunks[x]->recieveNewData(mEdgeBufferChunks[mChunkGridSize + (x + 1) / mChunkGridSize - 1].get());
	break;

	case edge::RIGHT:
		//Copy chunk data from right edge buffer into left of mChunks
		for (unsigned int z = (mChunkGridSize - 1) * mChunkGridSize; z < pow(mChunkGridSize, 2); z++)
			mChunks[z]->recieveNewData(mEdgeBufferChunks[mChunkGridSize * 2 + (mChunkGridSize - (z % mChunkGridSize)) - 1].get());
	break;

	case edge::BOTTOM:
		//Copy chunk data from bottom edge buffer into left of mChunks
		for (unsigned int x = 0; x <= (mChunkGridSize - 1) * mChunkGridSize; x += mChunkGridSize)
			mChunks[x]->recieveNewData(mEdgeBufferChunks[mChunkGridSize * 3 + (pow(mChunkGridSize, 2) - x) / mChunkGridSize - 1].get());
	break;

	default:
	break;
	}
}

void ChunkManager::updateBufferChunks(unsigned int edge) {
	switch (edge) {
	case LEFT:
		for (unsigned int z = 0; z < mChunkGridSize; z++) {
			mEdgeBufferChunks[z]->updateHeightsArray();
			mEdgeBufferChunks[z]->updateInnerNormals();
		}
		break;

	case TOP:
		for (int x = mChunkGridSize; x < mChunkGridSize * 2; x++) {
			mEdgeBufferChunks[x]->updateHeightsArray();
			mEdgeBufferChunks[x]->updateInnerNormals();
		}
		break;

	case RIGHT:
		for (int z = mChunkGridSize * 2; z < mChunkGridSize * 3; z++) {
			mEdgeBufferChunks[z]->updateHeightsArray();
			mEdgeBufferChunks[z]->updateInnerNormals();
		}
		break;

	case BOTTOM:
		for (int x = mChunkGridSize * 3; x < mChunkGridSize * 4; x++) {
			mEdgeBufferChunks[x]->updateHeightsArray();
			mEdgeBufferChunks[x]->updateInnerNormals();
		}
		break;

	default:
		Framework::Utils::ConsoleLogger::log(Framework::Utils::ConsoleLogger::LogType::ERROR_HALT, "Cannot update buffer chunk '" + std::to_string(edge) + "'. Must be within edge enum (0, 1, 2, 3)");
		break;
	}
}

void ChunkManager::instantiateSharedResources() {
	using namespace Framework::Graphics;

	mResourceSet->addTexture2D("src/Resources/Textures/martian_soil.png", "chunkTexture");
	mChunkTexture = mResourceSet->getResource<Texture2D>("chunkTexture");

	mResourceSet->addShader("src/Resources/Shaders/3D/ChunkVertex.shader", "src/Resources/Shaders/3D/ChunkFragment.shader", "chunkShader");
	mChunkShader = mResourceSet->getResource<Shader>("chunkShader");
	
	mResourceSet->addVertexBuffer(GL_STATIC_DRAW, VertexFormat(0, 2, GL_FLOAT, false), std::vector<float>(), "chunkXZBuffer");
	mXZChunkPositionData = mResourceSet->getResource<VertexBuffer>("chunkXZBuffer");

	mResourceSet->addVertexBuffer(GL_STATIC_DRAW, VertexFormat(2, 2, GL_FLOAT, false), std::vector<float>(), "chunkTextureCoords");
	mChunkTextureCoords = mResourceSet->getResource<VertexBuffer>("chunkTextureCoords");

	for (unsigned short i = 0; i < mNumLODs; i++) {
		mResourceSet->addIndexBuffer(GL_STATIC_DRAW, std::vector<unsigned int>(), "chunkLODIndexBuffers" + std::to_string(i));
		mLODIndexBuffers.push_back(mResourceSet->getResource<IndexBuffer>("chunkLODIndexBuffers" + std::to_string(i)));
	}
}

void ChunkManager::loadSharedResources() {
	int a = Chunk::mMaxNumSquares;
	
	//Loads the shared chunk-XZ-position VBO with data
	std::vector<float> tempXZData;
	for (int x = -a / 2; x <= a / 2; x++) {
		for (int z = -a / 2; z <= a / 2; z++) {
			tempXZData.push_back(x);
			tempXZData.push_back(z);
		}
	}
	mXZChunkPositionData->updateData(tempXZData);

	//Loads the shared chunk-texture-coordinate VBO with data 
	std::vector<float> tempTexCoordData;
	for (unsigned int u = 0; u <= a; u++) {
		for (unsigned int v = 0; v <= a; v++) {
			tempTexCoordData.push_back(static_cast<float>(u) / a);
			tempTexCoordData.push_back(static_cast<float>(v) / a);
		}
	}
	mChunkTextureCoords->updateData(tempTexCoordData);

	//Loads the shared IBO vector of different LODs with data
	unsigned short LODinterval = 0;
	unsigned int baseIndex = 0;
	for (unsigned int i = 0; i < mLODIndexBuffers.size(); i++) {
		std::vector<unsigned int> tempIndexData;
		LODinterval = 0.5 * (pow(2, i + 1));

		for (unsigned int x = 0; x < a; x += LODinterval) {
			for (unsigned int z = 0; z < a; z += LODinterval) {
				baseIndex = x * (a + 1) + z;

				tempIndexData.push_back(baseIndex);
				tempIndexData.push_back(baseIndex + LODinterval * (a + 1));
				tempIndexData.push_back(baseIndex + LODinterval * (a + 1) + LODinterval);
				tempIndexData.push_back(tempIndexData.back());
				tempIndexData.push_back(baseIndex + LODinterval);
				tempIndexData.push_back(baseIndex);
			}
		}

		mLODIndexBuffers[i]->updateData(tempIndexData);
	}

	initShaderUniforms();
}

void ChunkManager::initShaderUniforms() {
	mChunkShader->addUniform("modelMatrix");
	mChunkShader->addUniform("viewMatrix");
	mChunkShader->addUniform("projectionMatrix");
	mChunkShader->addUniform("sunDirection"); //3
	mChunkShader->addUniform("sunCol");	 //4
	mChunkShader->addUniform("skyColour");    //5
	mChunkShader->addUniform("usesFog");		 //6
	mChunkShader->addUniform("fogDensity");	 //7
	mChunkShader->addUniform("fogGradient");	 //8

	mChunkShader->bind();

	glm::vec3 sunDirection = -glm::normalize(mEnvironment->getSun().getPosition());
	mChunkShader->setUniform(3, sunDirection);
	mChunkShader->setUniform(4, mEnvironment->getSun().getColour());
	mChunkShader->setUniform(5, mEnvironment->getSkyColour());	//0.5098039215686275f, 0.4588235294117647f, 0.3254901960784314f
	mChunkShader->setUniform(6, true);
	mChunkShader->setUniform(7, mEnvironment->getFogDensity());
	mChunkShader->setUniform(8, mEnvironment->getFogGradient());
}