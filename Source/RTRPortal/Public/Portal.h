// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPortal, Log, All);


UCLASS()
class RTRPORTAL_API APortal : public AActor
{
	GENERATED_BODY()

public:

	/* The portal mesh. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Portal")
		class UPortalStaticMeshComponent* portalMesh;

	/* Box overlap component for teleporting actors.
	 * NOTE: Used instead of overlaps with the portalMesh because the physics doesn't call overlaps 
	 * if the mesh passed through something that doesn't block.
	 * This also still happens with CCD enabled as 
	 * its mainly for blocking physics between frames not calling overlaps... */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Portal")
		class UBoxComponent* portalBox;

	/* Scene capture component for creating the portal render target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
		class UPortalSceneCaptureComponent2D* portalCapture;

	/* The other portal actor to target. */
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Portal")
		class AActor* cTargetPortal;

	/* The max number of times a portal can recurse any portals. NOTE: Only target portal is supported for now. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
		int recursionAmount;

	/* The percentage of the screen resolution to render the portal at. */
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal", meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	//	float resolutionPercentile;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Portal")
		TArray<AActor*> cPortalList;

	//UPROPERTY(BlueprintReadWrite, VisibleDefaultsOnly, Category = "Portal")
	//	AActor* playerCopy;

	/* Debug the duplicated camera position and rotation relative 
	to the other portal by drawing debug cube based of scenecapture2D transform. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal|Debugging")
		bool debugCameraTransform;

	/* Log when a new actor is added to the trackedActors map and when one is removed. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal|Debugging")
		bool debugTrackedActors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal|Debugging")
		bool debugIsInfront;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal|Debugging")
		bool debugPortalMeshRect;

	/* Portal class target portal. */
	UPROPERTY()
		APortal* pTargetPortal;		

	
public:	
	// Sets default values for this actor's properties
	APortal();

protected:
	/* The player controller. */
	UPROPERTY()
		class APlayerController* portalController;

	UPROPERTY()
		class ULocalPlayer* portalPlayer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Delayed setup function. */
	UFUNCTION()
		void Setup();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/* Called when the portal box is overlapped. */
	UFUNCTION(Category = "Portal")
		void OnPortalBoxOverlapStart(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit);

	/* Called when the portal box ends one of its overlap events. */
	UFUNCTION(Category = "Portal")
		void OnPortalBoxOverlapEnd(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex);

	/* Called when the portal mesh ends one of its overlap events. */
	UFUNCTION(Category = "Portal")
		void OnPortalMeshOverlapStart(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit);

	/* Called when the portal mesh ends one of its overlap events. */
	UFUNCTION(Category = "Portal")
		void OnPortalMeshOverlapEnd(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex);

	/* Called when viewport resized */
	void OnViewportResized(FViewport* viewport, UINT32 index);


private:

	bool initialised; /* Has begin play been ran. */
};
