// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSceneCaptureComponent2D.h"
#include "RTRPortalBPLibrary.h"
#include "PortalStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


UPortalSceneCaptureComponent2D::UPortalSceneCaptureComponent2D()
{
	bEnableClipPlane = true;
	bUseCustomProjectionMatrix = true;
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	LODDistanceFactor = 1;
	ShowFlags.SetPostProcessing(false);
	ShowFlags.SetBloom(false);
	ShowFlags.SetPostProcessMaterial(false);
	ShowFlags.SetBSP(false);
	ShowFlags.SetScreenSpaceReflections(false);
	ShowFlags.SetScreenSpaceAO(false);
	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	PostProcessBlendWeight = 0;
}

void UPortalSceneCaptureComponent2D::UpdateRenderTarget(int viewportX, int viewportY, 
	int width, int height)
{
	//UE_LOG(LogPortal, Log, TEXT("%s before rectHeight: %d, rectWidth: %d"), *GetDebugName(this),
	//	rectHeight, rectWidth);

	TextureTarget = renderTarget;

	if (!renderTarget || renderTarget->GetFName().IsNone())
		return;

	if (width <= 0 || height <= 0)
		return;

	if (width > viewportX || height > viewportY)
		return;

	renderTarget->ResizeTarget(width, height);
}

//void UPortalSceneCaptureComponent2D::CreateRenderTarget(int width, int height)
//{
//	// Create new RTT
//	renderTarget = NewObject<UTextureRenderTarget2D>(this, 
//		UTextureRenderTarget2D::StaticClass(), FName("Default texture"));
//
//	renderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
//	renderTarget->Filter = TextureFilter::TF_Bilinear;
//	renderTarget->SizeX = width;
//	renderTarget->SizeY = height;
//	renderTarget->ClearColor = FLinearColor::Blue;
//	renderTarget->TargetGamma = 2.2f;
//	renderTarget->bNeedsTwoCopies = false;
//	renderTarget->AddressX = TextureAddress::TA_Clamp;
//	renderTarget->AddressY = TextureAddress::TA_Clamp;
//	renderTarget->NeverStream = true;
//
//	// Not needed since the texture is displayed on screen directly
//	// in some engine versions this can even lead to crashes (notably 4.24/4.25)
//	renderTarget->bAutoGenerateMips = false;
//
//	// This force the engine to create the render target 
//	// with the parameters we defined just above
//	renderTarget->UpdateResource();
//
//	TextureTarget = renderTarget;
//
//}

FMinimalViewInfo UPortalSceneCaptureComponent2D::UpdateSceneCapture(ULocalPlayer* localPlayer, 
	const FMinimalViewInfo& viewInfo, APortal* portal, const FIntRect& portalMeshRect, bool debugCameraTransform)
{
	FMinimalViewInfo sceneCpatureInfo (viewInfo);
	if (!portal || !portal->pTargetPortal)
		return sceneCpatureInfo;

	bEnableClipPlane = true;
	bOverride_CustomNearClippingPlane = true;
	ClipPlaneNormal = portal->pTargetPortal->portalMesh->GetUpVector();
	ClipPlaneBase = portal->pTargetPortal->portalMesh->GetComponentLocation();
	bUseCustomProjectionMatrix = true;

	//Update Transform
	FVector newCameraLocation = URTRPortalBPLibrary::ConvertLocationToPortal(viewInfo.Location, portal,
		portal->pTargetPortal);
	FRotator newCameraRotation = URTRPortalBPLibrary::ConvertRotationToPortal(viewInfo.Rotation, portal,
		portal->pTargetPortal);

	SetWorldLocationAndRotation(newCameraLocation, newCameraRotation);

	// Use-full for debugging convert transform to target function on the camera.
	if (debugCameraTransform)
	{
		DrawDebugCamera(GetWorld(), newCameraLocation,
			newCameraRotation, FOVAngle, 1.0f, FColor::Red, false, 20.0f);
		DrawDebugLine(GetWorld(), portal->portalMesh->GetComponentLocation(), newCameraLocation, FColor::Red, false, 20.0f);
		//UE_LOG(LogPortal, Log, TEXT("%s SceneCapture location: %s, rotation: %s"),
		//	*GetDebugName(this), *newCameraLocation.ToString(), *newCameraRotation.ToString());
	}

	//Update view info
	int32 viewportX, viewportY = 0;
	if (localPlayer && localPlayer->ViewportClient)
	{
		FVector2D ViewportSize;
		localPlayer->ViewportClient->GetViewportSize(ViewportSize);

		viewportX = ViewportSize.X;
		viewportY = ViewportSize.Y;
	}

	float rectHeight = portalMeshRect.Max.Y - portalMeshRect.Min.Y;
	float rectWidth = portalMeshRect.Max.X - portalMeshRect.Min.X;

	GetCameraView(0, sceneCpatureInfo);

	if (!bCaculateCustomProjectMatrix) 
	{
		CustomProjectionMatrix = viewInfo.CalculateProjectionMatrix();
		TextureTarget = renderTarget;
		return sceneCpatureInfo;
	}
	if (rectWidth <= 0 || rectHeight <= 0 || 
		(rectWidth > FMath::TruncToFloat(viewportX) && rectHeight > FMath::TruncToFloat(viewportY)))
		return sceneCpatureInfo;

	FVector2D portalMeshMid (portalMeshRect.Min.X + rectWidth / 2, portalMeshRect.Min.Y + rectHeight / 2);

	//FOVAngle
	float defaultFOV = viewInfo.FOV;
	float desiredFOV = FMath::RadiansToDegrees(
		FMath::Atan(FMath::Tan(FMath::DegreesToRadians(defaultFOV / 2)) *
			rectWidth / FMath::TruncToFloat(viewportX)) * 2);
	desiredFOV = desiredFOV < defaultFOV ? desiredFOV : defaultFOV;
	FOVAngle = desiredFOV;
	LODDistanceFactor = desiredFOV / FMath::Max<float>(0.01f, viewInfo.FOV);

	//Because of AspectRatio, update RenderTarget before GetCameraView
	UpdateRenderTarget(viewportX, viewportY, rectWidth, rectHeight);

	GetCameraView(0, sceneCpatureInfo);

	//OffCenterProjectionOffset
	FVector2D origin = FVector2D(FMath::TruncToFloat(viewportX) / 2,
		FMath::TruncToFloat(viewportY) / 2);
	FVector2D Offset = portalMeshMid - origin;
	FVector2D centerOffset(2 * Offset.X / rectWidth, -2 * Offset.Y / rectHeight);
	sceneCpatureInfo.OffCenterProjectionOffset = centerOffset;

	//CustomProjectionMatrix
	TOptional<FMatrix> CustomMatrix;
	FMatrix ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
	UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(sceneCpatureInfo, 
		CustomMatrix, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
	CustomProjectionMatrix = ProjectionMatrix;

	return sceneCpatureInfo;
}