// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalStaticMeshComponent.h"
#include "HelperMacros.h"
#include "RTRPortalBPLibrary.h"
#include "Logging/LogMacros.h"
#include "DrawDebugHelpers.h"
#include "RTRPortalDebugHUD.h"

DEFINE_LOG_CATEGORY(LogPortalMesh);


UPortalStaticMeshComponent::UPortalStaticMeshComponent() 
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_Portal);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CastShadow = false; // Dont want it casting shadows.

	//Use Default Plane ( Four Vertices )
	auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	if (MeshAsset.Object)
		SetStaticMesh(MeshAsset.Object);
}

TMap<FIntPoint, FVertex>* UPortalStaticMeshComponent::GetMeshVertices() const
{
	if (meshVertices.Num() == 0)
		CacheStaticMeshVertics();

	return &meshVertices;
}

FPlane UPortalStaticMeshComponent::GetPortaMeshPlane()
{
	return FPlane(GetComponentLocation(), GetUpVector());
}

bool UPortalStaticMeshComponent::CalculatePortalMeshRect(ULocalPlayer* localPlayer, 
	const FMinimalViewInfo viewInfo, FIntRect& outRect, bool debugPoint)
{
	CHECK_DESTROY_COMP(LogPortalMesh, !localPlayer,
		"CalculatePortalMeshRect: local player class couldn't be found in the portalMesh %s.", *GetName());

	CHECK_DESTROY_COMP(LogPortalMesh, !localPlayer->ViewportClient,
		"CalculatePortalMeshRect: local player class couldn't be found in the portalMesh %s.", *GetName());


	//FSceneViewProjectionData ProjectionData;
	//if (localPlayer->GetProjectionData(localPlayer->ViewportClient->Viewport, 
	//	eSSP_FULL, /*inout*/ ProjectionData) == false)
	//	return false;
	FVector2D ViewportSize;
	localPlayer->ViewportClient->GetViewportSize(ViewportSize);

	if (!URTRPortalBPLibrary::IsComponentInCameraViewFrustum(this, viewInfo))
		return false;

	int count = 0;
	for (auto& meshVertexKV : meshVertices) 
	{
		FVertex meshVertex = meshVertexKV.Value;
		FVector2D pixelPoint;
		bool flag = localPlayer->GetPixelPoint(meshVertex.WorldPosition, pixelPoint);

		meshVertex.IsInView = 
			URTRPortalBPLibrary::IsPointInCameraViewFrustum(meshVertex.WorldPosition, viewInfo) && flag;

		pixelPoint.X = FMath::Clamp(pixelPoint.X, 0.0f, ViewportSize.X);
		pixelPoint.Y = FMath::Clamp(pixelPoint.Y, 0.0f, ViewportSize.Y);

		meshVertex.PixelPosition = pixelPoint;

		if (count == 0)
			outRect.Min = FIntPoint(pixelPoint.X, pixelPoint.Y);

		if (count == 0)
			outRect.Max = FIntPoint(pixelPoint.X, pixelPoint.Y);

		outRect.Min.X = meshVertex.PixelPosition.X < outRect.Min.X ? meshVertex.PixelPosition.X : outRect.Min.X;
		outRect.Min.Y = meshVertex.PixelPosition.Y < outRect.Min.Y ? meshVertex.PixelPosition.Y : outRect.Min.Y;

		outRect.Max.X = meshVertex.PixelPosition.X > outRect.Max.X ? meshVertex.PixelPosition.X : outRect.Max.X;
		outRect.Max.Y = meshVertex.PixelPosition.Y > outRect.Max.Y ? meshVertex.PixelPosition.Y : outRect.Max.Y;

		count++;

		//if (debugPoint && meshVertex.Index.X > meshVertex.Index.Y) 
		//	DrawDebugPoint(GetWorld(), meshVertex.WorldPosition, 10, FColor::Red);

		//if (debugPoint && meshVertex.Index.X < meshVertex.Index.Y)
		//	DrawDebugPoint(GetWorld(), meshVertex.WorldPosition, 10, FColor::Red);
	}

	if (debugPoint)
		if(auto debugHUD = Cast<ARTRPortalDebugHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
			debugHUD->DebugRects.Add(outRect);

	if (outRect.Max.X < outRect.Min.X || outRect.Max.Y < outRect.Min.Y)
		return false;

	return true;
}

void UPortalStaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheStaticMeshVertics();

	cPortalMaterial = CreateDynamicMaterialInstance(0, cPortalMaterialInstance);

}

void UPortalStaticMeshComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void UPortalStaticMeshComponent::CacheStaticMeshVertics() const
{
	meshVertices.Empty(4);

	meshMidPoint = FVector::ZeroVector;

	// Vertex Buffer
	if (!IsValidLowLevel()) return;

	UStaticMesh* staticMesh = GetStaticMesh();
	if (!staticMesh) return;

	FStaticMeshRenderData* renderData = GetStaticMesh()->GetRenderData();
	if (!renderData) return;

	FStaticMeshLODResources& LOD_model = renderData->LODResources[0];
	uint32 numVertices = LOD_model.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();

	for (uint32 vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		const FVector& localPosition = LOD_model.VertexBuffers.PositionVertexBuffer.
			VertexPosition(vertexIndex);
		const FVector worldPosition = GetComponentTransform().TransformPosition(localPosition);

		auto vertex = FVertex(worldPosition);
		vertex.Index = FIntPoint(FMath::Sign(localPosition.X), FMath::Sign(localPosition.Y));

		meshVertices.Add(vertex.Index, vertex);

		meshMidPoint += worldPosition;
	}

	meshMidPoint /= numVertices;
}
