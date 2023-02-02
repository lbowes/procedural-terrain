#include "Player.h"
#include "Input/Input.h"
#include "Environment/Environment.h"

Player::Player(glm::vec3 position, float height) :
	mPosition(position),
	mHeight(height),
	mGazeDirection(glm::vec3(0.0f, 0.0f, -1.0f))
{
#if PLAYER_PERSPECTIVE
	mCamera = std::make_unique<Framework::PerspectiveCamera>(glm::vec3(0.0f, position.y + mHeight, 0.0f), mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f), 0.1f, 1000.0f, 0, 45.0f); //45.0f for perspective
#else
	mCamera = std::make_unique<Framework::OrthographicCamera>(glm::vec3(0.0f, position.y + mHeight, 0.0f), mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f), -1000.0f, 1000.0f, 0, 50.0f); //45.0f for perspective
#endif

	mCrouchedHeight = mHeight * 0.5f;  //0.5
	mCrouchedMaxSpeed = 0.2f;
}

void Player::update(double delta, Environment* e, float windowAspect) {
	//This then applies the velocity to the player's position, moving them (their gaze direction has already
	//been changed)
	move(delta, e);
	
	//And then these updated components are passed to the camera, to update its own internal components before 
	//constructing a view matrix for rendering

	mCamera->updatePositionOrientation(glm::vec3(mPosition.x, mPosition.y + mHeight, mPosition.z), mGazeDirection);
	mCamera->updateAspect(windowAspect);
}

void Player::manageInput(float delta) {
	using namespace Framework;

	updateGazeDirection();
	
	if (Input::isKeyPressed(GLFW_KEY_Q)) mFreeToFly = true;
	if (Input::isKeyPressed(GLFW_KEY_Z)) mFreeToFly = false;

	if (mFreeToFly) {
		if (Input::isKeyPressed(GLFW_KEY_W)) mVelocity += glm::normalize(glm::vec3(mGazeDirection.x, 0.0f, mGazeDirection.z)) * mAcceleration * delta;
		if (Input::isKeyPressed(GLFW_KEY_S)) mVelocity -= glm::normalize(glm::vec3(mGazeDirection.x, 0.0f, mGazeDirection.z)) * mAcceleration * delta;

		if (Input::isKeyPressed(GLFW_KEY_A)) mVelocity -= glm::normalize(glm::cross(mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * mAcceleration * delta;
		if (Input::isKeyPressed(GLFW_KEY_D)) mVelocity += glm::normalize(glm::cross(mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * mAcceleration * delta;

		if (Input::isKeyPressed(GLFW_KEY_SPACE)) mVelocity.y += mAcceleration * delta;
		if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)) mVelocity.y -= mAcceleration * delta;
	}
	else {
		if (!mInAir && mVelocity.length() < 10.0f) {
			if (Input::isKeyPressed(GLFW_KEY_W)) mVelocity += glm::normalize(glm::vec3(mGazeDirection.x, 0, mGazeDirection.z)) * mAcceleration * delta;
			if (Input::isKeyPressed(GLFW_KEY_S)) mVelocity -= glm::normalize(glm::vec3(mGazeDirection.x, 0, mGazeDirection.z)) * mAcceleration * delta;

			if (Input::isKeyPressed(GLFW_KEY_A)) mVelocity -= glm::normalize(glm::cross(mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * mAcceleration * delta;
			if (Input::isKeyPressed(GLFW_KEY_D)) mVelocity += glm::normalize(glm::cross(mGazeDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * mAcceleration * delta;
		}

		if (Input::isKeyPressed(GLFW_KEY_SPACE)) jump();
		if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)) crouch();
		else
			standUp();
	}

	updateCameraZoom();
}

void Player::move(float delta, Environment* e) {
	if (mFreeToFly) {
		mVelocity *= 0.94f;
	}
	else {
		mFrictionRatio = 1 / (1 + (delta * e->getFriction()));
		
		if (!mInAir) {
			mVelocity.x *= mFrictionRatio;
			mVelocity.z *= mFrictionRatio;
		}
		mVelocity.y = mUpwardsSpeed;
		
		mUpwardsSpeed += e->getGravity() * delta;
	}
	
	mPosition += mVelocity * delta;
	mPositionInWorld += mVelocity * delta;
	
	ChunkManager& chunkManager = e->getTerrain()->getChunkManager();
	int chunkSize = (chunkManager.getHighestLODChunkNumSquares());

	if (mPosition.x > chunkSize / 2) {
		chunkManager.updateChunksAlongX(1);
		mPosition.x = -chunkSize / 2;
	}
	else if (mPosition.x < -chunkSize / 2) {
		chunkManager.updateChunksAlongX(-1);
		mPosition.x = chunkSize / 2;
	}

	if (mPosition.z > chunkSize / 2) {
		chunkManager.updateChunksAlongZ(1);
		mPosition.z = -chunkSize / 2;
	}
	else if (mPosition.z < -chunkSize / 2) {
		chunkManager.updateChunksAlongZ(-1);
		mPosition.z = chunkSize / 2;
	}

	checkGroundCollision(e->getTerrain()->getGroundHeight(glm::vec2(mPosition.x, mPosition.z)));
}

void Player::updateGazeDirection() {
	mMouseDelta = Framework::Input::getMouseDelta();

	mFaceYaw += mMouseDelta.x * mLookAroundSensitivity;
	mFacePitch -= mMouseDelta.y * mLookAroundSensitivity;

	if (mFacePitch > 89.0f) mFacePitch = 89.0f;
	if (mFacePitch < -89.0f) mFacePitch = -89.0f;

	mGazeDirection.x = cos(glm::radians(mFacePitch)) * cos(glm::radians(mFaceYaw));
	mGazeDirection.y = sin(glm::radians(mFacePitch));
	mGazeDirection.z = cos(glm::radians(mFacePitch)) * sin(glm::radians(mFaceYaw));
	mGazeDirection = glm::normalize(mGazeDirection);
}

void Player::updateCameraZoom() {
	const float zoomSensitivity = 0.1f;
	
	float 
		currentFOV = mCamera->getFOVY(),
		mouseScroll = Framework::Input::getMouseScroll();

	if (currentFOV >= 44.0f && currentFOV <= 46.0f)
		mCamera->updateFOVY(currentFOV -= mouseScroll * zoomSensitivity);
	if (currentFOV <= 44.0f)
		mCamera->updateFOVY(44.0f);
	if (currentFOV >= 46.0f)
		mCamera->updateFOVY(46.0f);
}

void Player::jump() {
	if (!mInAir) {
		mUpwardsSpeed = mJumpingPower;
		mInAir = true;
	}
}

void Player::crouch() {
	if (!mInAir && !mIsCrouched) {
		mHeight *= mCrouchedHeight;
		mJumpingPower *= mCrouchedJumpPower;
		mAcceleration *= mCrouchedMaxSpeed;
		mIsCrouched = true;
	}
}

void Player::standUp() {
	if (mIsCrouched) {
		mHeight /= mCrouchedHeight;
		mJumpingPower /= mCrouchedJumpPower;
		mAcceleration /= mCrouchedMaxSpeed;
		mIsCrouched = false;
	}
}

void Player::setPosition(glm::vec3 offsetToCurrentPosition) {
	mPosition += offsetToCurrentPosition;
}

void Player::changeSpeed(float modifier, double delta) {
	mAcceleration += modifier * delta;
	if (mAcceleration > mMaxSpeed) mAcceleration = mMaxSpeed;
	else if (mAcceleration < mMinSpeed) mAcceleration = mMinSpeed;
}

void Player::checkGroundCollision(double terrainHeight) {
	if (mPosition.y < terrainHeight) {
		mUpwardsSpeed = 0.0f;
		mPosition.y = terrainHeight;
		mInAir = false;
	}
}