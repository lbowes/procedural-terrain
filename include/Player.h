#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include "Camera/PerspectiveCamera.h"
#include "Camera/OrthographicCamera.h"

class Terrain;
class Environment;

#define PLAYER_PERSPECTIVE true
#define FREE_CAMERA true

class Player {
private:
#if PLAYER_PERSPECTIVE
	std::unique_ptr<Framework::PerspectiveCamera> mCamera;
#else
	std::unique_ptr<Framework::OrthographicCamera> mCamera;
#endif

	glm::vec3 
		mVelocity,
		mPosition,
		mPositionInWorld,
		mGazeDirection,
		mHeadBobTranslation;

	glm::vec2 mMouseDelta;

	float
		mHeight,
		mMaxSpeed = 1000.0f,
		mMinSpeed = 10.0f,
		mCrouchedHeight,
		mCrouchedMaxSpeed = 0.2f,
		mCrouchedJumpPower = 0.4f,
		mFrictionRatio = 0.0f,
		mUpwardsSpeed,
		mJumpingPower = 15.0f, //15.0f when height is 1.785f
		mAcceleration = 30.0f,  //30.0f
		mLookAroundSensitivity = 0.08f; //0.08f

	bool
		mFreeToFly = true,
		mInAir = false,
		mIsCrouched = false;

	float 
		mFaceYaw = 0.0f,
		mFacePitch = 0.0f;

public:
	Player(glm::vec3 position, float height);
	~Player() = default;

#if PLAYER_PERSPECTIVE
	inline Framework::PerspectiveCamera* getCamera() { return mCamera.get(); }
#else 
	inline Framework::OrthographicCamera* getCamera() { return mCamera.get(); }
#endif
	inline glm::vec3& getPosition() { return mPosition; }
	inline glm::vec3& getPositionInWorld() { return mPositionInWorld; }
	inline glm::vec3& getGazeDirection() { return mGazeDirection; }

	void update(double delta, Environment* e, float windowAspect);
	void manageInput(float delta);
	void move(float delta, Environment* e);
	void updateGazeDirection();
	void updateCameraZoom();
	void jump();
	void crouch();
	void standUp();
	void setPosition(glm::vec3 newPosition);
	void changeSpeed(float modifier, double delta);

private:
	void checkGroundCollision(double terrainHeight);

};

#endif
