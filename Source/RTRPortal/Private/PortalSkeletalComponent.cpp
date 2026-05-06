// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSkeletalComponent.h"
#include "Animation/SkeletalMeshActor.h"

AActor* UPortalSkeletalComponent::Clone()
{
	auto parent = Cast<USkeletalMeshComponent>(GetAttachParent());
	if (!parent)
		return nullptr;
	auto actor = GetWorld()->SpawnActor<AActor>();
	auto skeletal = NewObject<USkeletalMeshComponent>(actor);
	skeletal->RegisterComponent();
	skeletal->SetRelativeTransform(parent->GetRelativeTransform());
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 7
	skeletal->SetSkeletalMeshAsset(parent->GetSkeletalMeshAsset());
#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 24
	skeletal->SkeletalMesh = parent->SkeletalMesh;
#endif
	skeletal->SetLeaderPoseComponent(parent);

	return actor;
}