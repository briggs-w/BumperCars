// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------


//GK this is now casuing errors because there's a dialoguebox defined in winuser.h that we are now using to get the model working with directx11 
//might just need to rename this

#include "MadhavDialogBox.h"
#include "Game.h"
#include "Renderer.h"
#include <SDL/SDL.h>
//namespace Madhav
//{
	MadhavDialogBox::MadhavDialogBox(Game* game, const std::string& text,
		std::function<void()> onOK)
		:UIScreen(game)
	{
		// Adjust positions for dialog box
		mBGPos = Vector2(0.0f, 0.0f);
		mTitlePos = Vector2(0.0f, 100.0f);
		mNextButtonPos = Vector2(0.0f, 0.0f);

		mBackground = mGame->GetRenderer()->GetTexture("Assets/DialogBG.png");
		SetTitle(text, Vector3::Zero, 30);
		AddButton("OKButton", [onOK]() {
			onOK();
			});
		AddButton("CancelButton", [this]() {
			Close();
			});
	}

	

	MadhavDialogBox::~MadhavDialogBox()
	{
	}

//}