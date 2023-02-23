// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalFunctionComponent.h"
#include "HelperMacros.h"
#include "PortalStaticMeshComponent.h"
#include "RTRPortalBPLibrary.h"



// Sets default values for this component's properties
UPortalFunctionComponent::UPortalFunctionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPortalFunctionComponent::BeginPlay()
{
	Super::BeginPlay();
	PrimaryComponentTick.SetTickFunctionEnable(false);
	FTimerHandle timer;
	FTimerDelegate timerDel;
	timerDel.BindUFunction(this, "SetupComponent");
	GetOwner()->GetWorldTimerManager().SetTimer(timer, timerDel, 0.1f, false, 0.0f);
	// ...
	
}

void UPortalFunctionComponent::SetupComponent()
{
	if (GetOwner()->GetFlags() != RF_BeingRegenerated)
		//clone owner
		ClonedActor = Clone();

	if (!ClonedActor)
		return;

	auto parent = Cast<UPrimitiveComponent>(GetAttachParent());
	if (!parent)
		return;

	if (!parent->GetGenerateOverlapEvents())
		parent->SetGenerateOverlapEvents(true);

	parent->SetCollisionResponseToChannel(ECC_Portal, ECollisionResponse::ECR_Overlap);
	parent->SetCollisionResponseToChannel(ECC_PortalBox, ECollisionResponse::ECR_Overlap);


	// If playing game and is game world setup delegate bindings.
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		// Bind the portals overlap events.
		parent->OnComponentBeginOverlap.AddDynamic(this, &UPortalFunctionComponent::OnAttachParentStartOverlap);
		parent->OnComponentEndOverlap.AddDynamic(this, &UPortalFunctionComponent::OnAttachParentEndOverlap);
	}

	PrimaryComponentTick.SetTickFunctionEnable(true);
}

AActor* UPortalFunctionComponent::Clone()
{
	return nullptr;
}


// Called every frame
void UPortalFunctionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ClonedActor)
		return;
	ClonedActor->SetActorLocationAndRotation(GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation());

	if (!OverlapedPortal)
		return;
	FVector currLocation = GetOwner()->GetActorLocation();
	FVector pointInterscetion;
	FVector dirPath = currLocation - LastTrackedOrigin;

	FPlane portalPlane = OverlapedPortal->portalMesh->GetPortaMeshPlane();
	bool passedThroughPortal = FMath::SegmentPlaneIntersection(LastTrackedOrigin,
		currLocation, portalPlane, pointInterscetion);
	bool checkEnterDirection = FVector::DotProduct(dirPath, OverlapedPortal->portalMesh->GetUpVector()) < 0;

	if (!passedThroughPortal || !checkEnterDirection)
		return;

	// Teleport the physics object. Teleport both position and relative velocity.
	FVector newLinearVelocity = URTRPortalBPLibrary::ConvertDirectionToTargetPortal(GetOwner()->GetVelocity(), OverlapedPortal);
	FVector convertedLoc = URTRPortalBPLibrary::ConvertLocationToPortal(GetOwner()->GetActorLocation(), OverlapedPortal, OverlapedPortal->pTargetPortal);
	FRotator convertedRot = URTRPortalBPLibrary::ConvertRotationToPortal(GetOwner()->GetActorRotation(), OverlapedPortal, OverlapedPortal->pTargetPortal);
	GetOwner()->SetActorLocationAndRotation(convertedLoc, convertedRot);
	GetOwner()->GetRootComponent()->ComponentVelocity = newLinearVelocity;

	ClonedActor->SetActorLocationAndRotation(URTRPortalBPLibrary::ConvertLocationToPortal(GetOwner()->GetActorLocation(), OverlapedPortal->pTargetPortal, OverlapedPortal),
		URTRPortalBPLibrary::ConvertRotationToPortal(GetOwner()->GetActorRotation(), OverlapedPortal->pTargetPortal, OverlapedPortal));

	// ...
}

void UPortalFunctionComponent::OnAttachParentStartOverlap(UPrimitiveComponent* triggeredComponent,
	AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit)
{
	auto portal = Cast<APortal>(overlappedActor);
	if (!portal)
		return;

	OverlapedPortal = portal;

	LastTrackedOrigin = GetOwner()->GetActorLocation();

	auto portalMesh = Cast<UPortalStaticMeshComponent>(overlappedComp);
	if (!portalMesh)
		return;

	ClonedActor->SetActorHiddenInGame(false);
}

void UPortalFunctionComponent::OnAttachParentEndOverlap(UPrimitiveComponent* triggeredComponent, AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex)
{
	auto portal = Cast<APortal>(overlappedActor);
	if (!portal)
		return;

	OverlapedPortal = nullptr;

	ClonedActor->SetActorHiddenInGame(true);

}

