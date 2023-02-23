// Fill out your copyright notice in the Description page of Project Settings.


#include "RTRPortalSubsystem.h"
#include "Portal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PortalSceneCaptureComponent2D.h"

DEFINE_LOG_CATEGORY(LogPortalSubsystem);



bool URTRPortalSubsystem::RegisterPortal(APortal* PortalToRegister)
{
	UE_LOG(LogPortalSubsystem, Warning, TEXT("RegisterPortal Begin"));
	if (!PortalToRegister)
		return false;
	Portals.AddUnique(PortalToRegister);
	PortalToRegister->OnEndPlay.AddDynamic(this, &URTRPortalSubsystem::OnActorEndPlay);
	UTextureRenderTarget2D* renderTarget (CreateRenderTarget());
	FreeRenderTargets.Add(renderTarget);
	return true;
}

bool URTRPortalSubsystem::UnregisterPortal(APortal* PortalToRemove)
{
	if (FreeRenderTargets.IsValidIndex(0))
		FreeRenderTargets.RemoveAt(0);
	if (PortalToRemove && Portals.Remove(PortalToRemove))
	{
		PortalToRemove->OnEndPlay.RemoveAll(this);
		return true;
	}
	return false;
}

int URTRPortalSubsystem::AssignRenderTarget(APortal* PortalToAssign)
{
	//if (PortalToAssign->portalCapture->GetRenderTarget())
	//	return -1;

	UTextureRenderTarget2D* renderTarget;
	if (!FreeRenderTargets.IsValidIndex(0))
		return -1;
	renderTarget = FreeRenderTargets[0];
	FreeRenderTargets.RemoveAt(0);

	if (!renderTarget)
		renderTarget = CreateRenderTarget();

	PortalToAssign->portalCapture->SetRenderTarget(renderTarget);
	
	return OccupiedRenderTargets.Add(renderTarget);
}

bool URTRPortalSubsystem::ReleaseRenderTarget(APortal* PortalAssigned)
{
	PortalAssigned->portalCapture->SetRenderTarget(nullptr);
	UTextureRenderTarget2D* renderTarget;
	if (!OccupiedRenderTargets.IsValidIndex(0))
		return false;
	renderTarget = OccupiedRenderTargets[0];
	OccupiedRenderTargets.RemoveAt(0);
	FreeRenderTargets.Add(renderTarget);
	return true;
}

void URTRPortalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogPortalSubsystem, Warning, TEXT(__FUNCTION__" [World] %-20s\t"), *GetLocalPlayer()->GetName());
}

void URTRPortalSubsystem::Deinitialize()
{
	// Ensure that the resources from the world are cleaned up and callbacks unregistered
	for (APortal* portal : Portals)
	{
		if (portal)
		{
			portal->OnEndPlay.RemoveAll(this);
		}
	}
	Portals.Empty();

	Super::Deinitialize();
}

bool URTRPortalSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{

	return Super::ShouldCreateSubsystem(Outer);
}

UTextureRenderTarget2D* URTRPortalSubsystem::CreateRenderTarget()
{
	int width, height;
	GetWorld()->GetFirstPlayerController()->GetViewportSize(width, height);

	// Create new RTT
	UTextureRenderTarget2D* outTarget = NewObject<UTextureRenderTarget2D>(this,
		UTextureRenderTarget2D::StaticClass());

	outTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	outTarget->Filter = TextureFilter::TF_Bilinear;
	outTarget->SizeX = width;
	outTarget->SizeY = height;
	outTarget->ClearColor = FLinearColor::Blue;
	outTarget->TargetGamma = 2.2f;
	outTarget->bNeedsTwoCopies = false;
	outTarget->AddressX = TextureAddress::TA_Clamp;
	outTarget->AddressY = TextureAddress::TA_Clamp;
	outTarget->NeverStream = true;

	// Not needed since the texture is displayed on screen directly
	// in some engine versions this can even lead to crashes (notably 4.24/4.25)
	outTarget->bAutoGenerateMips = false;

	// This force the engine to create the render target 
	// with the parameters we defined just above
	outTarget->UpdateResource();

	return outTarget;
}

void URTRPortalSubsystem::OnActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	UnregisterPortal(Cast<APortal>(Actor));
}
