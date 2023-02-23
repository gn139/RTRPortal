// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RTRPortalDebugHUD.generated.h"

/**
 * 
 */
UCLASS()
class RTRPORTAL_API ARTRPortalDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	TArray<FIntRect> DebugRects;

	/** The Main Draw loop for the hud.  Gets called before any messaging.  Should be subclassed */
	virtual void DrawHUD();
	
};
