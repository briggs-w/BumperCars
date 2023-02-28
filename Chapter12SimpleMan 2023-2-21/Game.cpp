// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------
// ----------------------------------------------------------------

//edited by Greg Kopack to add loading 3d models

#include "Game.h"
#include <algorithm>
#include "Renderer.h"
#include "AudioSystem.h"
#include "PhysWorld.h"
#include "Actor.h"
#include "UIScreen.h"
#include "HUD.h"
#include "MeshComponent.h"
#include "FollowActor.h"
#include "PlaneActor.h"
#include "TargetActor.h"
#include "BallActor.h"
#include "PauseMenu.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Font.h"
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include "Skeleton.h"
#include "Animation.h"
#include <chrono>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <iostream>

#include "AssimpPlusDemoApp.h"

//global
Game* instance_;

Game::Game(AssimpPlus::AssimpPlusDemoApp& app, HINSTANCE appHandle, LPCWSTR appName)
:mRenderer(nullptr)
,mAudioSystem(nullptr)
,mPhysWorld(nullptr)
,mGameState(EGameplay)
,mUpdatingActors(false)
, appPtr_ (&app)
//The DirectX Assimp stuff
, debugIcosphere_(nullptr)
, roadModel_(nullptr)
, manModel_(nullptr)
, materialOnlyShader_(nullptr)
, texturedShader_(nullptr)
, cameraDirectX_(Vector3(0.f, 2.2f, 0.f), Vector3(0.f, 2.2f, 1.f), Vector3::UnitY)
, projMatrixDirectX_(Matrix4::PerspectiveLH(Math::ToRadians(80.f), (app.windowSize_.right - app.windowSize_.left) / (float)(app.windowSize_.bottom - app.windowSize_.top), 0.1f, 100.f))
{
}

bool Game::Initialize()
{
	//These are needed for AppPlusDemoApp, and should go away eventually.
	if (!appPtr_->InitializeWin32()) { std::cerr << "Failed to initialize Win32! Exiting..." << std::endl; return false; }
	if (!appPtr_->InitializeD3D()) { std::cerr << "Failed to initialize D3D! Exiting..." << std::endl;	return false; }

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	// Create the renderer
	mRenderer = new Renderer(this);
	if (!mRenderer->Initialize(1024.0f, 768.0f)) //makes the screen
	{
		SDL_Log("Failed to initialize renderer");
		delete mRenderer;
		mRenderer = nullptr;
		return false;
	}

	// Create the audio system
	mAudioSystem = new AudioSystem(this);
	if (!mAudioSystem->Initialize())
	{
		SDL_Log("Failed to initialize audio system");
		mAudioSystem->Shutdown();
		delete mAudioSystem;
		mAudioSystem = nullptr;
		return false;
	}

	// Create the physics world
	mPhysWorld = new PhysWorld(this);
	
	// Initialize SDL_ttf
	if (TTF_Init() != 0)
	{
		SDL_Log("Failed to initialize SDL_ttf");
		return false;
	}

	LoadData();

	mTicksCount = SDL_GetTicks();

	return true;
}

void Game::RunLoop()
{
	while (mGameState != EQuit)
	{
		ProcessInput(); 
		UpdateGame();
		GenerateOutput();
	}
}

void Game::AddPlane(PlaneActor* plane)
{
	mPlanes.emplace_back(plane);
}

void Game::RemovePlane(PlaneActor* plane)
{
	auto iter = std::find(mPlanes.begin(), mPlanes.end(), plane);
	mPlanes.erase(iter);
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				mGameState = EQuit;
				break;
			// This fires when a key's initially pressed
			case SDL_KEYDOWN:
				if (!event.key.repeat)
				{
					if (mGameState == EGameplay)
					{
						HandleKeyPress(event.key.keysym.sym);
					}
					else if (!mUIStack.empty())
					{
						mUIStack.back()->
							HandleKeyPress(event.key.keysym.sym);
					}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (mGameState == EGameplay)
				{
					HandleKeyPress(event.button.button);
				}
				else if (!mUIStack.empty())
				{
					mUIStack.back()->
						HandleKeyPress(event.button.button);
				}
				break;
			default:
				break;
		}
	}
	
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (mGameState == EGameplay)
	{
		for (auto actor : mActors)
		{
			if (actor->GetState() == Actor::EActive)
			{
				actor->ProcessInput(state);
			}
		}
	}
	else if (!mUIStack.empty())
	{
		mUIStack.back()->ProcessInput(state);
	}
}

void Game::HandleKeyPress(int key)
{
	switch (key)
	{
	case SDLK_ESCAPE:
		// Create pause menu
		new PauseMenu(this);
		break;
	case '-':
	{
		// Reduce master volume
		float volume = mAudioSystem->GetBusVolume("bus:/");
		volume = Math::Max(0.0f, volume - 0.1f);
		mAudioSystem->SetBusVolume("bus:/", volume);
		break;
	}
	case '=':
	{
		// Increase master volume
		float volume = mAudioSystem->GetBusVolume("bus:/");
		volume = Math::Min(1.0f, volume + 0.1f);
		mAudioSystem->SetBusVolume("bus:/", volume);
		break;
	}
	case '1':
	{
		// Load English text
		LoadText("Assets/English.gptext");
		break;
	}
	case '2':
	{
		// Load Russian text
		LoadText("Assets/Russian.gptext");
		break;
	}
	case SDL_BUTTON_LEFT:
	{
		break;
	}
	default:
		break;
	}
}

void Game::UpdateGame()
{
	// Compute delta time
	// Wait until 16ms has elapsed since last frame
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}
	mTicksCount = SDL_GetTicks();

	if (mGameState == EGameplay)
	{
		for (auto item : items_) 
			item->Update(deltaTime); 

		// Update all actors
		mUpdatingActors = true;
		for (auto actor : mActors)
		{
			actor->Update(deltaTime);
		}
		mUpdatingActors = false;

		// Move any pending actors to mActors
		for (auto pending : mPendingActors)
		{
			pending->ComputeWorldTransform();
			mActors.emplace_back(pending);
		}
		mPendingActors.clear();

		// Add any dead actors to a temp vector
		std::vector<Actor*> deadActors;
		for (auto actor : mActors)
		{
			if (actor->GetState() == Actor::EDead)
			{
				deadActors.emplace_back(actor);
			}
		}

		// Delete dead actors (which removes them from mActors)
		for (auto actor : deadActors)
		{
			delete actor;
		}
	}

	// Update audio system
	mAudioSystem->Update(deltaTime);
	
	// Update UI screens
	for (auto ui : mUIStack)
	{
		if (ui->GetState() == UIScreen::EActive)
		{
			ui->Update(deltaTime);
		}
	}
	// Delete any UIScreens that are closed
	auto iter = mUIStack.begin();
	while (iter != mUIStack.end())
	{
		if ((*iter)->GetState() == UIScreen::EClosing)
		{
			delete *iter;
			iter = mUIStack.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void Game::GenerateOutput()
{
	mRenderer->Draw();
}

void Game::LoadData()
{
	// Load English text
	LoadText("Assets/English.gptext");

	// Create actors
	Actor* a = nullptr;
	Quaternion q;

	// Cube
	a = new Actor(this);
	a->SetPosition(Vector3(200.0f, 75.0f, 0.0f));
	a->SetScale(100.0f);
	q = Quaternion (Vector3::UnitY, -Math::PiOver2);
	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::Pi + Math::Pi / 4.0f));
	a->SetRotation(q);
	MeshComponent* mc = new MeshComponent(a);
	mc->SetMesh(mRenderer->GetMesh("Assets/Cube.gpmesh"));

	//Sphere
	a = new Actor(this);
	a->SetPosition(Vector3(200.0f, -75.0f, 0.0f));
	a->SetScale(3.0f);
	mc = new MeshComponent(a);
	mc->SetMesh(mRenderer->GetMesh("Assets/Sphere.gpmesh"));

	// Setup floor
	const float start = -1250.0f;
	const float size = 250.0f;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			//a = new PlaneActor(this);
			//a->SetPosition(Vector3(start + i * size, start + j * size, -100.0f));
		}
	}

	// Left/right walls
	q = Quaternion(Vector3::UnitX, Math::PiOver2);
	for (int i = 0; i < 10; i++)
	{
		/*a = new PlaneActor(this);
		a->SetPosition(Vector3(start + i * size, start - size, 0.0f));
		a->SetRotation(q);
		
		a = new PlaneActor(this);
		a->SetPosition(Vector3(start + i * size, -start + size, 0.0f));
		a->SetRotation(q);*/
	}

	q = Quaternion::Concatenate(q, Quaternion(Vector3::UnitZ, Math::PiOver2));
	// Forward/back walls
	for (int i = 0; i < 10; i++)
	{
		/*a = new PlaneActor(this);
		a->SetPosition(Vector3(start - size, start + i * size, 0.0f));
		a->SetRotation(q);

		a = new PlaneActor(this);
		a->SetPosition(Vector3(-start + size, start + i * size, 0.0f));
		a->SetRotation(q);*/
	}

	// Setup lights
	mRenderer->SetAmbientLight(Vector3(0.2f, 0.2f, 0.2f));
	DirectionalLight& dir = mRenderer->GetDirectionalLight();
	dir.mDirection = Vector3(0.0f, -0.707f, -0.707f);
	dir.mDiffuseColor = Vector3(0.78f, 0.88f, 1.0f);
	dir.mSpecColor = Vector3(0.8f, 0.8f, 0.8f);

	// UI elements
	mHUD = new HUD(this);
	

	// Start music
	mMusicEvent = mAudioSystem->PlayEvent("event:/Music");

	// Enable relative mouse mode for camera look
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// Make an initial call to get relative to clear out
	SDL_GetRelativeMouseState(nullptr, nullptr);

	// Different camera actors
	mFollowActor = new FollowActor(this);

	// Create target actors
	a = new TargetActor(this);
	a->SetPosition(Vector3(1450.0f, 0.0f, 100.0f));
	a = new TargetActor(this);
	a->SetPosition(Vector3(1450.0f, 0.0f, 400.0f));
	a = new TargetActor(this);
	a->SetPosition(Vector3(1450.0f, -500.0f, 200.0f));
	a = new TargetActor(this);
	a->SetPosition(Vector3(1450.0f, 500.0f, 200.0f));
	a = new TargetActor(this);
	a->SetPosition(Vector3(0.0f, -1450.0f, 200.0f));
	a->SetRotation(Quaternion(Vector3::UnitZ, Math::PiOver2));
	a = new TargetActor(this);
	a->SetPosition(Vector3(0.0f, 1450.0f, 200.0f));
	a->SetRotation(Quaternion(Vector3::UnitZ, -Math::PiOver2));

	//***** THIS IS ALL DIRECTX STUFF FOR NOW *********//
			//2021-11-12 WSB Note: The first parameter of MaterialOnlyShader::Material, 5 lines down, was 
		// a version of RED with the alpha set to 0.8f. But Madhav doesn't seem to use transparent colors,
		// and we're conforming to Madhav, so I'm taking the alpha out (and hoping it doesn't look bad).
	//const Vector3 Ones(1.f, 1.f, 1.f);
	materialOnlyShader_.setContext(appPtr_->context_);
	texturedShader_.setContext(appPtr_->context_);

	debugIcosphere_ = std::make_shared<AssimpPlus::IcosphereActor> (this);
	debugIcosphere_->SetPosition(Vector3(0.f, 1.8f, 6.f));
	debugIcosphere_->SetScale(2.0);

	roadModel_ = std::make_shared<AssimpPlus::RoadActor>(this);
	roadModel_->SetRotation(::Quaternion::Concatenate(	Quaternion(Vector3::UnitZ, Math::ToRadians(-90.f)),
														Quaternion(Vector3::UnitX, Math::ToRadians(-90.f))));

	manModel_ = std::make_shared<AssimpPlus::SimpleManActor> (this);
	manModel_->SetPosition(Vector3(0.f, 1.21f, 3.0f));
	manModel_->SetRotation(::Quaternion::Concatenate(::Quaternion(Vector3::UnitY, Math::ToRadians(180.f)), 
													 ::Quaternion(Vector3::UnitX, Math::ToRadians(90.f))));

	if (!manModel_)													throw CantLoad();
	if (!materialOnlyShader_.Initialize(appPtr_->device_).get())	throw CantLoad();
	if (!texturedShader_.Initialize(appPtr_->device_).get())		throw CantLoad();

	static const Vector3 PureWhite(1.0f, 1.0f, 1.0f);
	::DirectionalLight sun(Vector3::Normalize(Vector3(2.f, -1.6f, 3.f)), PureWhite, PureWhite);
	materialOnlyShader_.SetSunLight(sun);

	items_.push_back(manModel_); 
	items_.push_back(roadModel_); 
	items_.push_back(debugIcosphere_);
}

void Game::UnloadData()
{
	//2022-05-11 WSB I commented this out because it's crashing; I think this is coz
	// I have some actors now that are shared by Assimp demo. I hope when that's no longer
	// true the crash will go away and I can uncomment this
	
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	//while (!mActors.empty())
	//{
	//	delete mActors.back();
	//}

	// Clear the UI stack
	while (!mUIStack.empty())
	{
		delete mUIStack.back();
		mUIStack.pop_back();
	}

	if (mRenderer)
	{
		mRenderer->UnloadData();
	}

	// Unload fonts
	for (auto f : mFonts)
	{
		f.second->Unload();
		delete f.second;
	}

	// Unload skeletons
	for (auto s : mSkeletons)
	{
		delete s.second;
	}

	// Unload animations
	for (auto a : mAnims)
	{
		delete a.second;
	}
}

void Game::Shutdown()
{
	UnloadData();
	TTF_Quit();
	delete mPhysWorld;
	if (mRenderer)
	{
		mRenderer->Shutdown();
	}
	if (mAudioSystem)
	{
		mAudioSystem->Shutdown();
	}
	SDL_Quit();
}

void Game::AddActor(Actor* actor)
{
	// If we're updating actors, need to add to pending
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActors.emplace_back(actor);
	}
}

void Game::RemoveActor(Actor* actor)
{
	// Is it in pending actors?
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		// Swap to end of vector and pop off (avoid erase copies)
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	// Is it in actors?
	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
	{
		// Swap to end of vector and pop off (avoid erase copies)
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}

void Game::PushUI(UIScreen* screen)
{
	mUIStack.emplace_back(screen);
}

Font* Game::GetFont(const std::string& fileName)
{
	auto iter = mFonts.find(fileName);
	if (iter != mFonts.end())
	{
		return iter->second;
	}
	else
	{
		Font* font = new Font(this);
		if (font->Load(fileName))
		{
			mFonts.emplace(fileName, font);
		}
		else
		{
			font->Unload();
			delete font;
			font = nullptr;
		}
		return font;
	}
}


void Game::LoadText(const std::string& fileName)
{
	// Clear the existing map, if already loaded
	mText.clear();
	// Try to open the file
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		SDL_Log("Text file %s not found", fileName.c_str());
		return;
	}
	// Read the entire file to a string stream
	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	// Open this file in rapidJSON
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);
	if (!doc.IsObject())
	{
		SDL_Log("Text file %s is not valid JSON", fileName.c_str());
		return;
	}
	// Parse the text map
	const rapidjson::Value& actions = doc["TextMap"];
	for (rapidjson::Value::ConstMemberIterator itr = actions.MemberBegin();
		itr != actions.MemberEnd(); ++itr)
	{
		if (itr->name.IsString() && itr->value.IsString())
		{
			mText.emplace(itr->name.GetString(), 
				itr->value.GetString());
		}
	}
}

const std::string& Game::GetText(const std::string& key)
{
	static std::string errorMsg("**KEY NOT FOUND**");
	// Find this text in the map, if it exists
	auto iter = mText.find(key);
	if (iter != mText.end())
	{
		return iter->second;
	}
	else
	{
		return errorMsg;
	}
}

Skeleton* Game::GetSkeleton(const std::string& fileName)
{
	auto iter = mSkeletons.find(fileName);
	if (iter != mSkeletons.end())
	{
		return iter->second;
	}
	else
	{
		Skeleton* sk = new Skeleton();
		if (sk->Load(fileName))
		{
			mSkeletons.emplace(fileName, sk);
		}
		else
		{
			delete sk;
			sk = nullptr;
		}
		return sk;
	}
}

Animation* Game::GetAnimation(const std::string& fileName)
{
	auto iter = mAnims.find(fileName);
	if (iter != mAnims.end())
	{
		return iter->second;
	}
	else
	{
		Animation* anim = new Animation();
		if (anim->Load(fileName))
		{
			mAnims.emplace(fileName, anim);
		}
		else
		{
			delete anim;
			anim = nullptr;
		}
		return anim;
	}
}

//copied, still not 100% on how these work
//commenting out for now, think it was breaking cause the included library was wrong, do i need callback in this function?
//LRESULT CALLBACK Game::HandleWin32Message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
LRESULT Game::HandleWin32Message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Handle key presses
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': inputState_.W_Pressed = true; break;
		case 'A': inputState_.A_Pressed = true; break;
		case 'D': inputState_.D_Pressed = true; break;
		case 'S': inputState_.S_Pressed = true; break;
		case VK_LEFT: inputState_.Left_Pressed = true; break;
		case VK_RIGHT: inputState_.Right_Pressed = true; break;
		case VK_ESCAPE: PostQuitMessage(0); break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case 'W': inputState_.W_Pressed = false; break;
		case 'A': inputState_.A_Pressed = false; break;
		case 'D': inputState_.D_Pressed = false; break;
		case 'S': inputState_.S_Pressed = false; break;
		case VK_LEFT: inputState_.Left_Pressed = false; break;
		case VK_RIGHT: inputState_.Right_Pressed = false; break;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_QUIT || msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return EXIT_SUCCESS;
	}
	//commenting out for now, not sure if needed, 
	//think this is breaking other parts of the code by being commented out
	else if (instance_)
	{
		return instance_->HandleWin32Message(hWnd, msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
