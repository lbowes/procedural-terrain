#ifndef TERRAINGAME_H
#define TERRAINGAME_H
#pragma once

#include "Framework.h"
#include "Environment/Environment.h"
#include "Player/Player.h"

#define DEBUG 0
#define PERSPECTIVE_CAM 1	

class TerrainGame : public Framework::Application {
private:
	std::unique_ptr<Framework::Graphics::Renderer> mRenderer;
	std::unique_ptr<Player> mPlayer;
	std::unique_ptr<Environment> mEnvironment;
	std::unique_ptr<RenderableChunk> mTestChunk;

#if PERSPECTIVE_CAM
	std::unique_ptr<Framework::PerspectiveCamera> mFixedTestCamera;
#else
	std::unique_ptr<Framework::OrthographicCamera> mFixedTestCamera;
#endif

public:
	TerrainGame();
	~TerrainGame() = default;

	virtual void onLoad();
	virtual void onInputCheck();
	virtual void onUpdate();
	virtual void onRender();

	void manageInput();

};

#endif
