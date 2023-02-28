// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

//edited by Greg Kopack to add loading 3d models

#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include "Math.h"
#include "SoundEvent.h"
#include <SDL/SDL_types.h>

//needed for dx11 use
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

//other includes from indego code example
//#include "AssimpManModel.h"
#include "SimpleManActor.h"
#include "MaterialOnlyShader.h"
#include "DebugIcosphere.h"
#include "RoadActor.h"
#include "AssimpPlusDemoApp.h"

class Game
{
public:		
	class CantLoad {}; //Erase this soon -- Madhav uses return values not exceptions

	Game(AssimpPlus::AssimpPlusDemoApp& app, HINSTANCE appHandle, LPCWSTR appName = L"game name goes here");//replace text with game name
	bool Initialize();
	void RunLoop();
	void Shutdown();

	void AddActor(class Actor* actor);
	void RemoveActor(class Actor* actor);

	class Renderer* GetRenderer() { return mRenderer; }
	class AudioSystem* GetAudioSystem() { return mAudioSystem; }
	class PhysWorld* GetPhysWorld() { return mPhysWorld; }
	class HUD* GetHUD() { return mHUD; }
	
	// Manage UI stack
	const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }
	void PushUI(class UIScreen* screen);
	
	class FollowActor* GetPlayer() { return mFollowActor; }
	
	enum GameState
	{
		EGameplay,
		EPaused,
		EQuit
	};
	
	GameState GetState() const { return mGameState; }
	void SetState(GameState state) { mGameState = state; }
	
	class Font* GetFont(const std::string& fileName);

	void LoadText(const std::string& fileName);
	const std::string& GetText(const std::string& key);

	class Skeleton* GetSkeleton(const std::string& fileName);

	class Animation* GetAnimation(const std::string& fileName);
	
	// Game-specific
	void AddPlane(class PlaneActor* plane);
	void RemovePlane(class PlaneActor* plane);
	std::vector<class PlaneActor*>& GetPlanes() { return mPlanes; }


	// Win32 message handling, accessed by a global method
	//does making this virtual fix it???
	virtual LRESULT HandleWin32Message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void ProcessInput();
	void HandleKeyPress(int key);
	void UpdateGame();
	void GenerateOutput();
	void LoadData();
	void UnloadData();
	
	// All the actors in the game
	std::vector<class Actor*> mActors;
	std::vector<class UIScreen*> mUIStack;
	// Map for fonts
	std::unordered_map<std::string, class Font*> mFonts;
	// Map of loaded skeletons
	std::unordered_map<std::string, class Skeleton*> mSkeletons;
	// Map of loaded animations
	std::unordered_map<std::string, class Animation*> mAnims;

	// Map for text localization
	std::unordered_map<std::string, std::string> mText;
	// Any pending actors
	std::vector<class Actor*> mPendingActors;

	class Renderer* mRenderer;
	class AudioSystem* mAudioSystem;
	class PhysWorld* mPhysWorld;
	class HUD* mHUD;

	Uint32 mTicksCount;
	GameState mGameState;
	// Track if we're updating actors right now
	bool mUpdatingActors;

	// Game-specific code
	class FollowActor* mFollowActor;
	std::vector<class PlaneActor*> mPlanes;
	class SpriteComponent* mCrosshair;
	SoundEvent mMusicEvent;

	Matrix4 projMatrix_;

	struct
	{
	public:
		bool W_Pressed;
		bool A_Pressed;
		bool S_Pressed;
		bool D_Pressed;

		bool Left_Pressed;
		bool Right_Pressed;
	} inputState_;
	//end SimpleManActor variables

public:
	//Assimp-DirectX stuff
	AssimpPlus::AssimpPlusDemoApp* appPtr_; //it's public so Renderer can see it -- for now

	AssimpPlus::MaterialOnlyShader materialOnlyShader_;
	AssimpPlus::TexturedShader texturedShader_;

	AssimpPlus::FreeCamera cameraDirectX_;
	Matrix4 projMatrixDirectX_;	
	
	std::shared_ptr<AssimpPlus::IcosphereActor> debugIcosphere_;
	std::shared_ptr<AssimpPlus::RoadActor> roadModel_;
	std::shared_ptr<AssimpPlus::SimpleManActor> manModel_;
	std::vector<std::shared_ptr<DirectXThing>> items_;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

