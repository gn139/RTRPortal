// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Portal.h"
#include "PortalSceneCaptureComponent2D.generated.h"

/**
 * 
 */
UCLASS()
class RTRPORTAL_API UPortalSceneCaptureComponent2D : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PortalCapture")
		bool bCaculateCustomProjectMatrix = true;

public:
	UPortalSceneCaptureComponent2D();

	//--------Getter & Setter--------//
	
	FORCEINLINE UTextureRenderTarget2D* GetRenderTarget() const { return renderTarget; };

	FORCEINLINE void SetRenderTarget(UTextureRenderTarget2D* target) { renderTarget = target; };

	//------------------------------//

	FMinimalViewInfo UpdateSceneCapture(ULocalPlayer* localPlayer, const FMinimalViewInfo& viewInfo, 
		APortal* portal, const FIntRect& portalMeshRect, bool debugCameraTransform = false);

protected:
	/* The portals render target texture. */
	UPROPERTY()
		UTextureRenderTarget2D* renderTarget;

	void UpdateRenderTarget(int viewportX, int viewportY, int width, int height);

	//void CreateRenderTarget(int width, int height);

private:


};
