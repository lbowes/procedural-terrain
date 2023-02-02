#ifndef SUN_H
#define SUN_H
#pragma once

#include <glm/glm/vec3.hpp>
#include <glm/glm/geometric.hpp>
#include <glm/glm/gtx/rotate_vector.hpp>
#include <glm/glm/gtx/vector_angle.hpp>

class Sun {
private:
	glm::vec3 
		mPosition,
		mStartPosition,
		mDirection,
		mRotationAxis,
		mColour;

	float
		mRotation = 0.0f,
		mRotationRate = 0.0f,
		mAngleAboveHorizon = 0.0f;

public:
	Sun(glm::vec3 startingPosition, glm::vec3 rotationAxis, glm::vec3 startingColour);
	~Sun() = default;
	
	void update();
	void setRotation(float newRotation);
	
	inline glm::vec3 getPosition() const { return mPosition; }
	inline glm::vec3 getColour() const { return mColour; }
	inline glm::vec3 getDirection() const { return mDirection; }
	inline float getAngleAboveHorizon() const { return mAngleAboveHorizon; }

	inline void setColour(glm::vec3 newColour) { mColour = newColour; }

};

#endif