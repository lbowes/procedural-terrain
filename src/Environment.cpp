#include "Environment.h"
#include "Graphics/Renderer/Renderer.h"
#include "Player/Player.h"

#include <glm/glm/gtx/rotate_vector.hpp>

Environment::Environment(Framework::ResourceSet* r) :
	mSun(glm::vec3(20.0f, -20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.8f, 0.4f))
{
	//Sun colours
	//0.0f, 0.8f, 0.4f - green
	init(r);
}

void Environment::manageInput(float delta) {
	using namespace Framework;
	Graphics::Shader* tempShader = mTerrain->getChunkManager().getChunkShader();

	/*if (Input::isKeyPressed(GLFW_KEY_RIGHT)) {
		mSun.getPosition() = glm::rotate(mSun.getPosition(), glm::radians(mSunRotationSpeed) * delta, glm::vec3(1, 1, 0));
		tempShader->setUniform(3, -tempSunDirectionVec);
		mSkyBoxShader->bind();
		mSkyBoxShader->setUniform(3, tempSunDirectionVec);
	}
	else if (Input::isKeyPressed(GLFW_KEY_LEFT)) {
		mSun.getPosition() = glm::rotate(mSun.getPosition(), glm::radians(-mSunRotationSpeed) * delta, glm::vec3(1, 1, 0));
		tempShader->setUniform(3, -tempSunDirectionVec);
		mSkyBoxShader->bind();
		mSkyBoxShader->setUniform(3, tempSunDirectionVec);
	}*/

	if (Input::isKeyPressed(GLFW_KEY_PAGE_DOWN)) {
		if (mFogDensity < 0.015f)
			mFogDensity += 0.014f * delta;

		tempShader->setUniform(7, mFogDensity);
	}
	else if (Input::isKeyPressed(GLFW_KEY_PAGE_UP)) {
		if (mFogDensity > 0.00001f)
			mFogDensity -= 0.014f * delta;

		tempShader->setUniform(7, mFogDensity);
	}
}

void Environment::update(float delta, Player& player) {
	mTerrain->update(player);
	updateSky(delta);
}

void Environment::render(Framework::Graphics::Renderer& renderer) {
	mSkyBox->render(renderer);
	mTerrain->render(renderer);
}

void Environment::init(Framework::ResourceSet* r) {
	using namespace Framework::Graphics;

	mTerrain = std::make_unique<Terrain>(50, this, r);

	mTerrain->getChunkManager().getChunkShader()->bind();
	mTerrain->getChunkManager().getChunkShader()->setUniform(3, -mSun.getDirection());

	mSkyBox = std::make_unique<SkyBox>("src/Resources/Shaders/3D/skyboxVertex.shader", "src/Resources/Shaders/3D/skyboxFragment.shader");
	mSkyBoxShader = mSkyBox->getShader();
	mSkyBoxShader->addUniform("skyColour");
	mSkyBoxShader->addUniform("sunDir");
	mSkyBoxShader->addUniform("sunColour");
	
	mSkyBoxShader->bind();
	mSkyBoxShader->setUniform(2, mDaySkyColour);
	mSkyBoxShader->setUniform(3, mSun.getDirection());
	mSkyBoxShader->setUniform(4, mSun.getColour());
}

void Environment::updateSky(float delta) {
	mTime += mTimeMovementRate * delta;
	if (mTime > 1.0f)
		mTime = 0.0f;

	//Handles the changing time of day, the sun position, the sky colour etc
	mSun.setRotation(mTime);
	mSun.update();

	mSun.setColour(glm::vec3(1.0f, 0.0f, 0.0f));

	float angle = mSun.getAngleAboveHorizon();
	if (angle > 45.0f && angle > 45.0f) {
		mCurrentSkyColour = mDaySkyColour;
		mSunCurrentColour = mSunDayColour;
	}
	else if (angle < 45.0f && angle > -80.0f) {
		mCurrentSkyColour = glm::lerp(mNightSkyColour, mDaySkyColour, (angle + 80.0f) / 135.0f);
		mSunCurrentColour = glm::lerp(mSunNightColour, mSunDayColour, (angle + 80.0f) / 135.0f);
	}
	else {
		mCurrentSkyColour = mNightSkyColour;
		mSunCurrentColour = mSunNightColour;
	}

	updateShaderUniforms();
}
 
void Environment::updateShaderUniforms() {
	glm::vec3 tempSunDir = mSun.getDirection();

	Framework::Graphics::Shader* temp = mTerrain->getChunkManager().getChunkShader();
	temp->bind();
	temp->setUniform(3, -tempSunDir);
	temp->setUniform(4, mSunCurrentColour/*mSun.getColour()*/);
	temp->setUniform(5, mCurrentSkyColour);	//0.5098039215686275f, 0.4588235294117647f, 0.3254901960784314f

	mSkyBoxShader->bind();
	mSkyBoxShader->setUniform(2, mCurrentSkyColour);
	mSkyBoxShader->setUniform(3, tempSunDir);
	mSkyBoxShader->setUniform(4, mSunCurrentColour/*mSun.getColour()*/);
}