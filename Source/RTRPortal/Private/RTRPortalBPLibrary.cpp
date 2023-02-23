// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTRPortalBPLibrary.h"
#include "RTRPortal.h"
#include "Portal.h"
#include "PortalStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"



URTRPortalBPLibrary::URTRPortalBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

//float URTRPortalBPLibrary::RTRPortalSampleFunction(float Param)
//{
//	return -1;
//}

bool URTRPortalBPLibrary::IsActorInCameraViewFrustum(AActor* actor, const FMinimalViewInfo viewInfo, 
	const TOptional<FMatrix>& CustomProjectionMatrix)
{
	if (!actor)
		return false;

	FMatrix viewMatrix, projectionMatrix, viewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(viewInfo,
		viewMatrix, projectionMatrix, viewProjectionMatrix);

	if (CustomProjectionMatrix.IsSet())
		viewProjectionMatrix = viewMatrix * CustomProjectionMatrix.GetValue();

	// Get the camera frustum
	FConvexVolume frustum;
	GetViewFrustumBounds(frustum, viewProjectionMatrix, true);

	// Get the bounding box of the actor
	FVector origin, extent;
	actor->GetActorBounds(false, origin, extent);

	bool actorBoundsCheck = frustum.IntersectBox(origin, extent);

	return actorBoundsCheck;
}

bool URTRPortalBPLibrary::IsComponentInCameraViewFrustum(USceneComponent* component, 
	const FMinimalViewInfo viewInfo, const TOptional<FMatrix>& CustomProjectionMatrix)
{
	if (!component)
		return false;

	FMatrix viewMatrix, projectionMatrix, viewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(viewInfo,
		viewMatrix, projectionMatrix, viewProjectionMatrix);

	if (CustomProjectionMatrix.IsSet())
		viewProjectionMatrix = viewMatrix * CustomProjectionMatrix.GetValue();

	// Get the camera frustum
	FConvexVolume frustum;
	GetViewFrustumBounds(frustum, viewProjectionMatrix, true);

	// Get the bounding box of the actor
	FVector origin, extent;
	origin = component->Bounds.Origin;
	extent = component->Bounds.BoxExtent;

	bool componentBoundsCheck = frustum.IntersectBox(origin, extent);

	return componentBoundsCheck;
}

bool URTRPortalBPLibrary::IsPointInCameraViewFrustum(const FVector point, const FMinimalViewInfo viewInfo,
	const TOptional<FMatrix>& CustomProjectionMatrix)
{
	FMatrix viewMatrix, projectionMatrix, viewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(viewInfo,
		viewMatrix, projectionMatrix, viewProjectionMatrix);

	if (CustomProjectionMatrix.IsSet())
		viewProjectionMatrix = viewMatrix * CustomProjectionMatrix.GetValue();

	// Get the camera frustum
	FConvexVolume frustum;
	GetViewFrustumBounds(frustum, viewProjectionMatrix, true);

	bool pointCheck = frustum.IntersectPoint(point);

	return pointCheck;
}

FVector URTRPortalBPLibrary::ConvertDirectionToTargetPortal(const FVector& direction, 
	const APortal* currentPortal)
{
	// Flip the given direction to the target portal.
	FVector flippedVel;
	flippedVel.X = FVector::DotProduct(direction, currentPortal->portalMesh->GetUpVector());
	flippedVel.Y = FVector::DotProduct(direction, currentPortal->portalMesh->GetForwardVector());
	flippedVel.Z = FVector::DotProduct(direction, currentPortal->portalMesh->GetRightVector());
	FVector newVelocity = flippedVel.X * -currentPortal->pTargetPortal->portalMesh->GetUpVector()
		+ flippedVel.Y * -currentPortal->pTargetPortal->portalMesh->GetForwardVector()
		+ flippedVel.Z * currentPortal->pTargetPortal->portalMesh->GetRightVector();

	// Return flipped vector for the given direction.
	return newVelocity;
}

FVector URTRPortalBPLibrary::ConvertLocationToPortal(const FVector& location, 
	const APortal* currentPortal, const APortal* endPortal, bool flip)
{
	// Convert location to new portal.
	//FVector posRelativeToPortal = 
	//	currentPortal->portalMesh->GetComponentTransform().InverseTransformPosition(location);
	//if (flip)
	//{
	//	posRelativeToPortal.X *= -1; // Flip forward axis.
	//	posRelativeToPortal.Y *= -1; // Flip right axis.
	//}
	//FVector newWorldLocation = 
	//	endPortal->portalMesh->GetComponentTransform().TransformPosition(posRelativeToPortal);

	if (!currentPortal || !endPortal)
		return FVector::ZeroVector;

	FVector dir = location - currentPortal->portalMesh->GetComponentLocation();
	FVector TargetLocation = endPortal->portalMesh->GetComponentLocation();

	FVector NewDirection = ConvertDirectionToTargetPortal(dir, currentPortal);

	// Return the new location.
	return TargetLocation + NewDirection;

}

FRotator URTRPortalBPLibrary::ConvertRotationToPortal(const FRotator& rotation, 
	const APortal* currentPortal, const APortal* endPortal, bool flip)
{
	if (!currentPortal || !endPortal)
		return FRotator::ZeroRotator;

	FRotator temp = rotation;
	if (flip)
	{
		temp.Yaw += 180.0f;
	}

	 //Convert rotation to new portal.
	FRotator relativeRotation = 
		currentPortal->portalMesh->GetComponentTransform().
		InverseTransformRotation(temp.Quaternion()).Rotator();


	FRotator newWorldRotation = 
		endPortal->portalMesh->GetComponentTransform().
		TransformRotation(relativeRotation.Quaternion()).Rotator();


	//FTransform SourceTransform = currentPortal->portalMesh->GetComponentTransform();
	//FTransform TargetTransform = endPortal->portalMesh->GetComponentTransform();
	//FQuat QuatRotation = FQuat(rotation);

	//QuatRotation.Inverse();

	//FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * QuatRotation;
	//FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;
	// Return the new rotation.
	return newWorldRotation;

}

bool URTRPortalBPLibrary::IsInfront(const FVector& location, const APortal* currentPortal, bool debug)
{
	FVector direction = (location - currentPortal->portalMesh->GetComponentLocation()).GetSafeNormal();
	if (debug) 
	{
		DrawDebugLine(currentPortal->GetWorld(), currentPortal->portalMesh->GetComponentLocation(), 
			currentPortal->portalMesh->GetComponentLocation() + direction * 100, FColor::Red, true);
		DrawDebugLine(currentPortal->GetWorld(), currentPortal->portalMesh->GetComponentLocation(), 
			currentPortal->portalMesh->GetComponentLocation() + currentPortal->portalMesh->GetUpVector() * 100, FColor::Blue, true);
	}
	float dotProduct = FVector::DotProduct(direction, currentPortal->portalMesh->GetUpVector());//StaticMesh'/Engine/BasicShapes/Plane.Plane' face same to UpVector
	return (dotProduct >= 0); // Returns true if the location is in-front of this portal.
}
