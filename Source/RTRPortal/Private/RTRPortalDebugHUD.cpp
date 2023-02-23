// Fill out your copyright notice in the Description page of Project Settings.


#include "RTRPortalDebugHUD.h"

void ARTRPortalDebugHUD::DrawHUD()
{

	for (auto DebugRect : DebugRects) 
	{
		DrawRect(FLinearColor(1.f, 0, 0, 0.5f),
			DebugRect.Min.X, DebugRect.Min.Y, DebugRect.Width(), DebugRect.Height());
		DrawRect(FLinearColor::Blue, DebugRect.Max.X, DebugRect.Max.Y, 20, 20);
	}


	DebugRects.Empty();
}
