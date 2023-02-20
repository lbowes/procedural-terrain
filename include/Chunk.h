#ifndef CHUNK_H
#define CHUNK_H
#pragma once


#define FLAT_TERRAIN 0

class Chunk {
	friend class ChunkManager;

protected:
	static const int mMaxNumSquares = 16; //16
	int mXPositionInWorld = 0;
	int mZPositionInWorld = 0;
	float mHighestPoint = 0.0f;
	float mLowestPoint = 0.0f;

	std::vector<float> mHeights, mNormals;

public:
	Chunk(int positionInWorldX, int positionInWorldZ);

	void updateHeightsArray();
	void updateInnerNormals();
	void recieveNewData(Chunk* from);

	void changePosition(int offsetX, int offsetZ);
	bool insideSameLOD();

	inline std::vector<float>& getHeightsArray() { return mHeights; }
	static unsigned int getMaxNumSquares() { return mMaxNumSquares; }
	double getPerlinValue(int target);
	double getTerrainHeight(int x, int z);

};

#endif
