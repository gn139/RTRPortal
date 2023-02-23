// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Portal.h"
#include "PortalFunctionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RTRPORTAL_API UPortalFunctionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPortalFunctionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/* Delayed setup function. */
	UFUNCTION()
		virtual	void SetupComponent();

	virtual AActor* Clone();

protected:
	UPROPERTY()
		AActor* ClonedActor;

	UPROPERTY()
		APortal* OverlapedPortal;

	FVector LastTrackedOrigin;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Called when the portal box is overlapped. */
	UFUNCTION(Category = "Portal")
		virtual	void OnAttachParentStartOverlap(UPrimitiveComponent* triggeredComponent, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit);

	/* Called when the portal box ends one of its overlap events. */
	UFUNCTION(Category = "Portal")
		virtual	void  OnAttachParentEndOverlap(UPrimitiveComponent* triggeredComponent, AActor* overlappedActor, UPrimitiveComponent* overlappedComp,
			int32 otherBodyIndex);

private:
		
};
