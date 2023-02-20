#include "Chunk.h"
#include "Maths/Noise.h"
#include "Utils/ConsoleLogger.h"
#include "Utils/Timer.h"

#include <glm/glm/vec3.hpp>
#include <glm/glm/mat4x4.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <functional>


const int Chunk::mMaxNumSquares;


Chunk::Chunk(int positionInWorldX, int positionInWorldZ) :
	mXPositionInWorld(positionInWorldX),
	mZPositionInWorld(positionInWorldZ)
{
	mHeights.resize((mMaxNumSquares + 1) * (mMaxNumSquares + 1));
	mNormals.resize((mMaxNumSquares + 1) * (mMaxNumSquares + 1) * 3);

	updateHeightsArray();
	updateInnerNormals();
}


void Chunk::updateHeightsArray() {
	unsigned int xElement = 0, zElement = 0, mainIndex = 0;

	for (int x = mXPositionInWorld - mMaxNumSquares / 2; x <= mXPositionInWorld + mMaxNumSquares / 2; x++) {
		for (int z = mZPositionInWorld - mMaxNumSquares / 2; z <= mZPositionInWorld + mMaxNumSquares / 2; z++) {
			xElement = (x - mXPositionInWorld + (mMaxNumSquares / 2)) * (mMaxNumSquares + 1);
			zElement = (z - mZPositionInWorld + (mMaxNumSquares / 2));
			mainIndex = xElement + zElement;

			mHeights[mainIndex] = getTerrainHeight(x, z);

			if (mHeights[mainIndex] > mHighestPoint)
				mHighestPoint = mHeights[mainIndex];

			if (mHeights[mainIndex] < mLowestPoint)
				mLowestPoint = mHeights[mainIndex];
		}
	}
}


void Chunk::updateInnerNormals() {
	glm::vec3 normal(0.0f);
	double lowerLeft, left, lower, upper, upperRight, right, thisHeight;
	unsigned int currentElement = 0;

	//Go through all points that ONLY depend on other points IN THIS CHUNK (the edge points need data from neighbouring chunks. They are
	//handled separately after this to avoid lots of if statements in this loop).
	for (int x = 1; x < mMaxNumSquares; x++) {
		for (int z = 1; z < mMaxNumSquares; z++) {

			currentElement = x * (mMaxNumSquares + 1) + z;
			//Get the height of this point
			thisHeight = mHeights[currentElement];

			//Get 6 heights around this point
			lowerLeft = mHeights[currentElement - (mMaxNumSquares + 1) - 1];
			left = mHeights[currentElement - (mMaxNumSquares + 1)];
			lower = mHeights[currentElement - 1];
			upper = mHeights[currentElement + 1];
			upperRight = mHeights[currentElement + (mMaxNumSquares + 1) + 1];
			right = mHeights[currentElement + (mMaxNumSquares + 1)];

			//At this point, we have all the heights of the relevant neighbouring squares
			//Now, the normals have to be calculated for each of these neighbour points (to get an average at the end)
			normal += glm::cross(glm::vec3(0, lowerLeft, -1), glm::vec3(1, thisHeight, 0)) +
				glm::cross(glm::vec3(0, thisHeight, 1), glm::vec3(-1, lowerLeft, 0)) +
				glm::cross(glm::vec3(0, upper, 1), glm::vec3(-1, left, 0)) +
				glm::cross(glm::vec3(0, thisHeight, -1), glm::vec3(1, upperRight, 0)) +
				glm::cross(glm::vec3(0, upperRight, 1), glm::vec3(-1, thisHeight, 0)) +
				glm::cross(glm::vec3(0, lower, -1), glm::vec3(1, right, 0));

			//The final average normal for the specified vertex inside the chunk, has been calculated
			normal = glm::normalize(normal);

			//This normal is then added to the normal vector of this chunk, coordinate by coordinate
			mNormals[currentElement * 3] = normal.x;
			mNormals[currentElement * 3 + 1] = normal.y;
			mNormals[currentElement * 3 + 2] = normal.z;
		}
	}
}


void Chunk::recieveNewData(Chunk* from) {
	mHeights = from->mHeights;
	mNormals = from->mNormals;
	mHighestPoint = from->mHighestPoint;
	mLowestPoint = from->mLowestPoint;
}


void Chunk::changePosition(int offsetChunkPositionX, int offsetChunkPositionZ) {
	mXPositionInWorld += offsetChunkPositionX * mMaxNumSquares;
	mZPositionInWorld += offsetChunkPositionZ * mMaxNumSquares;
}


bool Chunk::insideSameLOD() {
#if 0
	if ((abs(mXPosition / mMaxNumSquares) != (mManagerForThisChunk.mChunkGridSize - 1) / 2) && (abs(mZPosition / mMaxNumSquares) != (mManagerForThisChunk.mChunkGridSize - 1) / 2)) {
		int gridSize = mManagerForThisChunk.mChunkGridSize;
		int currentChunkIndex = (((mXPosition / mMaxNumSquares) + floor(gridSize / 2)) * gridSize) + ((mZPosition / mMaxNumSquares) + floor(gridSize / 2));

		int leftLOD = mManagerForThisChunk.mChunks[currentChunkIndex - gridSize]->mLOD,
			upperLeftLOD = mManagerForThisChunk.mChunks[currentChunkIndex - gridSize + 1]->mLOD,
			upperLOD = mManagerForThisChunk.mChunks[currentChunkIndex + 1]->mLOD,
			upperRightLOD = mManagerForThisChunk.mChunks[currentChunkIndex + gridSize + 1]->mLOD,
			rightLOD = mManagerForThisChunk.mChunks[currentChunkIndex + gridSize]->mLOD,
			lowerRightLOD = mManagerForThisChunk.mChunks[currentChunkIndex + gridSize - 1]->mLOD,
			lowerLOD = mManagerForThisChunk.mChunks[currentChunkIndex - 1]->mLOD,
			lowerLeftLOD = mManagerForThisChunk.mChunks[currentChunkIndex - gridSize - 1]->mLOD;

		if (mLOD != leftLOD || mLOD != rightLOD || mLOD != upperLOD || mLOD != lowerLOD || mLOD != upperLeftLOD || mLOD != upperRightLOD || mLOD != lowerLeftLOD || mLOD != lowerRightLOD)
			return false;
		else
			return true;
	}
	else
		return false;
#endif
	return false;
}


double Chunk::getPerlinValue(int target) {
	double dChunkWidth = static_cast<double>(mMaxNumSquares);

	double wayThroughChunk = (target % mMaxNumSquares) / dChunkWidth;
	double floor = (target - (target % mMaxNumSquares)) / dChunkWidth;

	return floor + wayThroughChunk;
}


double Chunk::getTerrainHeight(int x, int z) {
#if !FLAT_TERRAIN
#if 0
	using namespace Framework::Maths;

	double
		perlinCorrectX = getPerlinValue(x),
		perlinCorrectZ = getPerlinValue(z),
		total = 0,
		amplitude = 0,
		frequency = 0,
		addition = 0;

	//Huge flat undulating terrain height
	//amplitude = 3; //5.4
	//frequency = 0.1; //0.01
	//addition = pow(Noise::Perlin(perlinCorrectX * frequency, perlinCorrectZ * frequency) * amplitude, 3);
	//total += addition;

	amplitude = 2.14;
	frequency = 0.05;
	addition = -pow(Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0, 0.7, 3) * amplitude, 3);
	//if (addition > 23) addition = 20;
	total += abs(addition);

	//Low ledges
	amplitude = 8;
	frequency = 0.1;
	addition = pow(Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0f, 0.5, 2) * amplitude, 2);
	if (addition > 3) addition = 3;
	total += addition * 0.78;

	//Piles of sand
	amplitude = 30;
	frequency = 1;
	addition = -Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0f, 1, 1) * amplitude;
	if (addition > 10 || addition < -10) addition = (addition / amplitude) * 1.0;
	else addition = 0;
	total += addition;

	return total;
#else
	using namespace Framework::Maths;

	double
		perlinCorrectX = getPerlinValue(x),
		perlinCorrectZ = getPerlinValue(z),
		total = 0,
		amplitude = 0,
		frequency = 0,
		addition = 0;

	//Huge flat undulating terrain height
	//amplitude = 3; //5.4
	//frequency = 0.1; //0.01
	//addition = pow(Noise::Perlin(perlinCorrectX * frequency, perlinCorrectZ * frequency) * amplitude, 3);
	//total += addition;

	amplitude = 4.0f;
	frequency = 0.05;
	addition = -pow(Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0, 0.7, 3) * amplitude, 3);
	//if (addition > 23) addition = 20;
	total += abs(addition);

	//Low ledges
	amplitude = 8;
	frequency = 0.1;
	addition = pow(Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0f, 0.5, 2) * amplitude, 2);
	if (addition > 3) addition = 3;
	total += addition * 0.78;

	//Piles of sand
	amplitude = 30;
	frequency = 1;
	addition = -Noise::multiFractalRidged(perlinCorrectX * frequency, perlinCorrectZ * frequency, 2.0f, 1, 1) * amplitude;
	if (addition > 10 || addition < -10) addition = (addition / amplitude) * 1.0;
	else addition = 0;
	total += addition;

	return total;
#endif
#else
	return 0;
#endif
}