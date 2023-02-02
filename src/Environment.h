#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#pragma once

#include "Terrain.h"
#include "Graphics/SkyBox.h"
#include "Sun.h"

#include <glm/glm/gtx/compatibility.hpp>

class Environment {
private:
	std::unique_ptr<Terrain> mTerrain;
	std::unique_ptr<Framework::Graphics::SkyBox> mSkyBox;
	Framework::Graphics::Shader* mSkyBoxShader = nullptr;
	Sun mSun;

	//0.0f, 0.4f, 0.3f dark turqoise
	//0.4431372549019608f, 0.407843137254902f, 0.2941176470588235f //mars sky
	glm::vec3 
		mCurrentSkyColour,
		mDaySkyColour = glm::vec3(0.0f, 0.4f, 0.3f),
		mNightSkyColour = glm::vec3(0.0f, 0.1, 0.075f);

	//temp
	glm::vec3
		mSunCurrentColour,
		mSunDayColour = glm::vec3(0.0f, 0.8f, 0.4f),
		mSunNightColour = glm::vec3(0.0f, 0.1f, 0.24f);
	//

	float 
		mTime = 0.0f,
		mTimeMovementRate = 0.005f,
		mFogDensity = 0.0059f,
		mFogGradient = 2.0f,
		mFriction = 10.0f,	//10.0f
		mGravity = -38.0f,	//-38.0f
		mSunRotationSpeed = 200.0f; //40.0f

public:
	Environment(Framework::ResourceSet* r);
	~Environment() = default;

	void manageInput(float delta);
	void update(float delta, Player& player);
	void render(Framework::Graphics::Renderer& renderer);

	inline Terrain* getTerrain() { return mTerrain.get(); }
	inline Sun& getSun() { return mSun; }
	inline glm::vec3& getSkyColour() { return mCurrentSkyColour; }
	inline float getFogDensity() { return mFogDensity; }
	inline float getFogGradient() { return mFogGradient; }
	inline float getFriction() { return mFriction; }
	inline float getGravity() { return mGravity; }

private:
	void init(Framework::ResourceSet* r);
	void updateSky(float delta);
	void updateShaderUniforms();

};

#endif
