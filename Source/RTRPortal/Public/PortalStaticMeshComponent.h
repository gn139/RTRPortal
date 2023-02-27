// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PortalStaticMeshComponent.generated.h"

/* Logging category for this class. */
DECLARE_LOG_CATEGORY_EXTERN(LogPortalMesh, Log, All);

/* Structure to hold important information with each Vertex */
USTRUCT(BlueprintType)
struct FVertex
{
	GENERATED_BODY()

public:

	FVector GamePosition;
	FIntPoint Index;
	FVector2D PixelPosition;
	bool IsInView;

public:

	/* Default Constructor. */
	FVertex()
	{
		GamePosition = FVector::ZeroVector;
		Index = FIntPoint::NoneValue;
		PixelPosition = FVector2D::ZeroVector;
		IsInView = false;
	}

	/* Main Constructor. */
	FVertex(FVector worldPosition)
	{
		GamePosition = worldPosition;
		Index = FIntPoint::NoneValue;
		PixelPosition = FVector2D::ZeroVector;
		IsInView = false;
	}
};

/**
 * 
 */
UCLASS()
class RTRPORTAL_API UPortalStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	/* The portals dynamic material instance. */
	UPROPERTY(/*BlueprintReadWrite, EditDefaultsOnly, Category = "Portal"*/)
		class UMaterialInstanceDynamic* cPortalMaterial;

	/* The portal material instance to create the dynamic material */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "PortalMesh")
		class UMaterialInterface* cPortalMaterialInstance;

public:
	UPortalStaticMeshComponent();

	// ---- Getters & Setters ---- //

	FORCEINLINE TMap<FIntPoint, FVertex>* GetMeshVertices() const;

	FORCEINLINE FVector GetMidPoint() const { return meshMidPoint; };

	FORCEINLINE FPlane GetPortaMeshPlane();
	// --------------------------- //


	//Get a rect for Protal Camera
	bool CalculatePortalMeshRect(ULocalPlayer* localPlayer, 
		const FMinimalViewInfo& viewInfo, FIntRect& outRect, bool debugPoint = false);
	
private:


protected:

	// Shortcuts to prevent loading every time, but mutable because it's just a shortcut
	mutable TMap<FIntPoint, FVertex> meshVertices;
	mutable FVector meshMidPoint;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void CacheStaticMeshVertics() const;

};
