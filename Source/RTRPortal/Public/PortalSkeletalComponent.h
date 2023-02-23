// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PortalFunctionComponent.h"
#include "PortalSkeletalComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RTRPORTAL_API UPortalSkeletalComponent : public UPortalFunctionComponent
{
	GENERATED_BODY()

protected:
	virtual AActor* Clone() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
		class AActor* ActorToClone;
	
};
