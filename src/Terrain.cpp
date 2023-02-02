#include "Terrain.h"
#include "Player/Player.h"
#include "Maths/Maths.hpp"
#include "Graphics/Renderer/Renderer.h"		

Terrain::Terrain(unsigned int chunkGridSize, Environment* e, Framework::ResourceSet* r) :
	mChunkGridSize(chunkGridSize),
	mChunkManager(chunkGridSize, e, r)
{ }

void Terrain::update(Player& player) {
	glm::vec3 front = player.getGazeDirection();
	float a = static_cast<float>(mChunkManager.getChunk(0).getMaxNumSquares()) * 2;
				
	mChunkManager.cullChunks(front, player.getCamera()->getPosition() - glm::normalize(glm::vec3(front.x, 0, front.z)) * a * 2.0f, player.getCamera());
}

void Terrain::render(Framework::Graphics::Renderer& renderer) {
	mChunkManager.renderChunks(renderer);
}
													 
double Terrain::getGroundHeight(glm::vec2& positionInChunk) {
	double xCoord = positionInChunk.x - (floor(positionInChunk.x));
	double zCoord = positionInChunk.y - (floor(positionInChunk.y));
	
	RenderableChunk* playerChunk = &mChunkManager.getPlayerChunk();

	glm::vec3 point1, point2, point3;
	glm::vec2 playerPos;

	playerPos.x = xCoord;
	playerPos.y = zCoord;

	unsigned int a = Chunk::getMaxNumSquares() / 2;
	unsigned int baseElement = 0;
	
	baseElement = ((floor(positionInChunk.x) + a) * (Chunk::getMaxNumSquares() + 1) + (floor(positionInChunk.y) + a));

	if (xCoord > zCoord) {
		point1.x = 0;
		point1.y = playerChunk->getHeightsArray()[baseElement];
		point1.z = 0;

		point2.x = 1;
		point2.y = playerChunk->getHeightsArray()[baseElement + Chunk::getMaxNumSquares() + 1];
		point2.z = 0;

		point3.x = 1;
		point3.y = playerChunk->getHeightsArray()[baseElement + Chunk::getMaxNumSquares() + 2];
		point3.z = 1;
	}
	else {
		point1.x = 0;
		point1.y = playerChunk->getHeightsArray()[baseElement];
		point1.z = 0;

		point2.x = 0;
		point2.y = playerChunk->getHeightsArray()[baseElement + 1];
		point2.z = 1;

		point3.x = 1;
		point3.y = playerChunk->getHeightsArray()[baseElement + Chunk::getMaxNumSquares() + 2];
		point3.z = 1;
	}

	return Framework::Maths::barryCentric(point1, point2, point3, playerPos);
}
