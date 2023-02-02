#include "TerrainGame.h"

TerrainGame::TerrainGame() :
	Application("Planet Generator")
{
	onLoad();
}

void TerrainGame::onInputCheck() {
	manageInput();
	mPlayer->manageInput(mFrameTime);
	mEnvironment->manageInput(mFrameTime);
}

void TerrainGame::onLoad() {
	using namespace Framework;

#if PERSPECTIVE_CAM
	mFixedTestCamera = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(0.0f, -1.0f, -0.001f), glm::vec3(0.0f, 1.0f, 0.0f), 0.01f, 1700.0f, mWindow.getAspect(), 45.0f);
#else
	mFixedTestCamera = std::make_unique<OrthographicCamera>(glm::vec3(0.0f, 600.0f, 0.0f), glm::vec3(0.0f, -1.0f, -0.001f), 0.0f, 1000.0f, mWindow.getAspect(), 450.0f);
#endif
	
	//Setting up miscellaneous stuff
	//Input::hideCursor();
	//Maths::Noise::initPermutations(); //temp, why can this not be called here without some access violation?
	mEnvironment = std::make_unique<Environment>(&mResourceSet);

	mWindow.setClearColour(glm::vec4(mEnvironment->getSkyColour(), 1.0f));
	
	mPlayer = std::make_unique<Player>(glm::vec3(0.0f, 0.0f, 0.0f), 1.785f); //1.785 for height
	mPlayer->getCamera()->updateAspect(mWindow.getAspect());

	mRenderer = std::make_unique<Graphics::Renderer>(mPlayer->getCamera());
}				  

void TerrainGame::onUpdate() {
	mEnvironment->update(mUpdateDelta, *mPlayer);

	float newAspect = mWindow.getAspect();
	mPlayer->update(mUpdateDelta, mEnvironment.get(), newAspect);
	mFixedTestCamera->updateAspect(newAspect);
}

void TerrainGame::onRender() {
	mEnvironment->render(*mRenderer);
	ImGui::Value("mouse x: ", Framework::Input::getMousePosition().x);
	ImGui::SameLine();
	ImGui::Value("mouse y: ", Framework::Input::getMousePosition().y);

	mRenderer->flush();
}

void TerrainGame::manageInput() {
	using namespace Framework;

	if (Input::isKeyPressed(GLFW_KEY_ESCAPE))
		mWindow.close();
	
	//temp
	//Many of these should be encapsulated in their own class.manageInput functions rather than doing it all in here.
	//These are just temporary anyway.
	if (Input::isKeyPressed(GLFW_KEY_UP)) {
		mPlayer->changeSpeed(100, mFrameTime);
	}
	if (Input::isKeyPressed(GLFW_KEY_DOWN)) {
		mPlayer->changeSpeed(-100, mFrameTime);
	}

	//temp
	//switching cameras
	if (Input::isKeyPressed(GLFW_KEY_1)) {
		mRenderer->setCamera(*mPlayer->getCamera());
	}
	if (Input::isKeyPressed(GLFW_KEY_2)) {
		mRenderer->setCamera(*mFixedTestCamera.get());
	}

	//toggle wireframe view
	if (Input::isKeyPressed(GLFW_KEY_9))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (Input::isKeyPressed(GLFW_KEY_0))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}