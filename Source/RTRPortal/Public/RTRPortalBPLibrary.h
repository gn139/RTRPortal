// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Portal.h"
#include "RTRPortalBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class URTRPortalBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		//UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "RTRPortal sample test testing"), Category = "RTRPortalTesting")
		//static float RTRPortalSampleFunction(float Param);

	static bool IsActorInCameraViewFrustum(AActor* actor, const FMinimalViewInfo viewInfo,
			const TOptional<FMatrix>& CustomProjectionMatrix = TOptional<FMatrix>());

	static bool IsComponentInCameraViewFrustum(USceneComponent* component, const FMinimalViewInfo viewInfo,
		const TOptional<FMatrix>& CustomProjectionMatrix = TOptional<FMatrix>());

	static bool IsPointInCameraViewFrustum(const FVector point, const FMinimalViewInfo viewInfo,
		const TOptional<FMatrix>& CustomProjectionMatrix = TOptional<FMatrix>());

	/* Convert a given velocity vector to the target portal. */
	UFUNCTION(BlueprintCallable, Category = "Portal")
		static FVector ConvertDirectionToTargetPortal(const FVector& direction, const APortal* currentPortal);

	/* Convert a given location to the target portal. */
	UFUNCTION(BlueprintCallable, Category = "Portal")
		static FVector ConvertLocationToPortal(const FVector& location, const APortal* currentPortal,
			const APortal* endPortal, bool flip = true);

	/* Convert a given rotation to the target portal. */
	UFUNCTION(BlueprintCallable, Category = "Portal")
		static FRotator ConvertRotationToPortal(const FRotator& rotation, const APortal* currentPortal,
			const APortal* endPortal, bool flip = true);

	/* Is the location in-front of this portal? */
	UFUNCTION(BlueprintCallable, Category = "Portal")
		static bool IsInfront(const FVector& location, const APortal* currentPortal, bool debug = false);
};
