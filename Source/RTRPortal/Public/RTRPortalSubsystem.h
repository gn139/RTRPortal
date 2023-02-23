// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RTRPortalSubsystem.generated.h"

class APortal;

/* Logging category for this class. */
DECLARE_LOG_CATEGORY_EXTERN(LogPortalSubsystem, Log, All);

/**
 * The RTRPortal Subsystem manage portals in World.
 */
UCLASS()
class RTRPORTAL_API URTRPortalSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	/**
	* Registers the given portal
	*
	* @return True if portal registered
	*/
	bool RegisterPortal(APortal* PortalToRegister);

	/**
	* Remove this portal from the array of Portals with this subsystem
	*
	* @return	True if this portal was removed
	*/
	bool UnregisterPortal(APortal* PortalToRemove);

	/**
	* Assign this portal a FreeRenderTarget
	*
	* @return	index of RenderTarget in OccupiedRenderTargets
	*/
	int AssignRenderTarget(APortal* PortalToAssign);

	/**
	* Release render target from this portal
	*
	* @return
	*/
	bool ReleaseRenderTarget(APortal* PortalAssigned);

protected:

	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	UTextureRenderTarget2D* CreateRenderTarget();

private:

	/** Callback for a registered actor's End Play so we can remove it from our known actors */
	UFUNCTION()
		void OnActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	/** Actors to check if they should auto destroy or not */
	UPROPERTY()
		TArray<APortal*> Portals;
	UPROPERTY()
		TArray<UTextureRenderTarget2D*> FreeRenderTargets;
	UPROPERTY()
		TArray<UTextureRenderTarget2D*> OccupiedRenderTargets;

};