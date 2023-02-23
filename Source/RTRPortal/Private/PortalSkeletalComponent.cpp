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
	skeletal->SkeletalMesh = parent->SkeletalMesh;
	skeletal->SetMasterPoseComponent(parent);

	return actor;
}

