#include "Sun.h"

Sun::Sun(glm::vec3 startingPosition, glm::vec3 rotationAxis, glm::vec3 startingColour) :
	mPosition(startingPosition),
	mStartPosition(startingPosition),
	mRotationAxis(rotationAxis),
	mColour(startingColour)
{ }

void Sun::update() {
	mPosition = glm::rotate(mStartPosition, glm::radians(mRotation * 360.0f), mRotationAxis);
	mDirection = glm::normalize(mPosition);

	if(mDirection.y > 0)
		mAngleAboveHorizon = glm::degrees(glm::angle(mDirection, glm::normalize(glm::vec3(mDirection.x, 0.0f, mDirection.z))));
	else
		mAngleAboveHorizon = -glm::degrees(glm::angle(mDirection, glm::normalize(glm::vec3(mDirection.x, 0.0f, mDirection.z))));
}

void Sun::setRotation(float newRotation) {
	if (newRotation < 0.0f)
		mRotation = 0.0f;
	else if (newRotation > 1.0f)
		mRotation = 1.0f;

	mRotation = newRotation;
}