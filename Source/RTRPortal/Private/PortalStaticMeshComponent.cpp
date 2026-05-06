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
#if WITH_EDITORONLY_DATA
	bEnableAutoLODGeneration = false;
#endif
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_Portal);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CastShadow = false; // Dont want it casting shadows.

	//Use Default Plane ( Four Vertices )
	auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	if (!MeshAsset.Object)
		return;
	SetStaticMesh(MeshAsset.Object);
	GetStaticMesh()->bAllowCPUAccess = true; //for CacheStaticMeshVertics in Package
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
	const FMinimalViewInfo& viewInfo, FIntRect& outRect, bool debugPoint)
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

	//FSceneViewProjectionData projectionData;
	//projectionData.SetViewRectangle(viewInfo.);

	int count = 0;
	float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
	for (auto& meshVertexKV : meshVertices) 
	{
		FVertex& meshVertex = meshVertexKV.Value;
		FVector2D pixelPoint;
		bool flag = localPlayer->GetPixelPoint(meshVertex.GamePosition, pixelPoint);

		meshVertex.IsInView = 
			URTRPortalBPLibrary::IsPointInCameraViewFrustum(meshVertex.GamePosition, viewInfo) && flag;

		pixelPoint.X = FMath::Clamp(pixelPoint.X, 0.0f, ViewportSize.X);
		pixelPoint.Y = FMath::Clamp(pixelPoint.Y, 0.0f, ViewportSize.Y);


		if (debugPoint)
			UE_LOG(LogPortalMesh, Warning, TEXT("%s"), *pixelPoint.ToString());

		meshVertex.PixelPosition = pixelPoint;

		if (count == 0)
		{
			minX = maxX = pixelPoint.X;
			minY = maxY = pixelPoint.Y;
		}
		else
		{
			minX = FMath::Min(minX, pixelPoint.X);
			minY = FMath::Min(minY, pixelPoint.Y);
			maxX = FMath::Max(maxX, pixelPoint.X);
			maxY = FMath::Max(maxY, pixelPoint.Y);
		}

		count++;

		if (debugPoint && meshVertex.Index.X > meshVertex.Index.Y) 
			DrawDebugPoint(GetWorld(), meshVertex.GamePosition, 10, FColor::Red);

		if (debugPoint && meshVertex.Index.X > meshVertex.Index.Y)
			if (auto debugHUD = Cast<ARTRPortalDebugHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
				debugHUD->DebugRects.Add(FIntRect(FIntPoint(pixelPoint.X, pixelPoint.Y), FIntPoint(pixelPoint.X + 20, pixelPoint.Y + 20)));

		if (debugPoint && meshVertex.Index.X < meshVertex.Index.Y)
			DrawDebugPoint(GetWorld(), meshVertex.GamePosition, 10, FColor::Red);

		if (debugPoint && meshVertex.Index.X < meshVertex.Index.Y)
			if (auto debugHUD = Cast<ARTRPortalDebugHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
				debugHUD->DebugRects.Add(FIntRect(FIntPoint(pixelPoint.X, pixelPoint.Y), FIntPoint(pixelPoint.X + 20, pixelPoint.Y + 20)));
	}

	outRect.Min = FIntPoint(FMath::FloorToInt(minX), FMath::FloorToInt(minY));
	outRect.Max = FIntPoint(FMath::CeilToInt(maxX), FMath::CeilToInt(maxY));

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
	//UE_LOG(LogPortalMesh, Warning, TEXT("Begin CacheStaticMeshVertics"));

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
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 7

		const FVector3f& localPosition =
			LOD_model
			.VertexBuffers
			.PositionVertexBuffer
			.VertexPosition(vertexIndex);

		const FVector3d worldPosition =
			GetComponentTransform()
			.TransformPosition(FVector3d(localPosition));

#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 24

		const FVector& localPosition = 
			LOD_model
			.VertexBuffers
			.PositionVertexBuffer
			.VertexPosition(vertexIndex);

		const FVector worldPosition = 
			GetComponentTransform().TransformPosition(localPosition);

#endif


		//UE_LOG(LogPortalMesh, Warning, TEXT("%s"), *worldPosition.ToString());
		auto vertex = FVertex(worldPosition);
		vertex.Index = FIntPoint(FMath::Sign(localPosition.X), FMath::Sign(localPosition.Y));

		meshVertices.Add(vertex.Index, vertex);

		meshMidPoint += worldPosition;
	}

	meshMidPoint /= numVertices;

	//UE_LOG(LogPortalMesh, Warning, TEXT("End CacheStaticMeshVertics"));

}
