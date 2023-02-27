// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include "PortalStaticMeshComponent.h"
#include "RTRPortalBPLibrary.h"
#include "Logging/LogMacros.h"
#include "HelperMacros.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "PortalSceneCaptureComponent2D.h"
#include "Engine/LocalPlayer.h"


DEFINE_LOG_CATEGORY(LogPortal);

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork; // After Camera Update

	// Create class default sub-objects.
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->Mobility = EComponentMobility::Static; // Portals are static objects.

	portalMesh = CreateDefaultSubobject<UPortalStaticMeshComponent>("PortalMesh");
	portalMesh->SetupAttachment(RootComponent);

	portalCapture = CreateDefaultSubobject<UPortalSceneCaptureComponent2D>("PortalCapture");
	portalCapture->SetupAttachment(RootComponent);

	// Setup portal overlap box for detecting actors.
	portalBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalBox"));
	portalBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	portalBox->SetUseCCD(true);
	portalBox->SetCollisionProfileName("Portal");
	portalBox->SetupAttachment(portalMesh);
}

void APortal::Setup()
{
	// If there is no target destroy and print log message.
	//UE_LOG(LogPortal, Log, TEXT("cTargetPortal is %s"), *GetDebugName(cTargetPortal));
	pTargetPortal = Cast<APortal>(cTargetPortal);
	CHECK_DESTROY_VOID(LogPortal, (!cTargetPortal || !pTargetPortal), "Portal %s, was destroyed as there was no target portal or it wasnt a type of APortal class.", *GetName());

	// Save a reference to the player controller.
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	CHECK_DESTROY_VOID(LogPortal, !PC, "Player controller could not be found in the portal class %s.", *GetName());
	portalController = PC;

	ACharacter* character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	CHECK_DESTROY_VOID(LogPortal, !character, "Player portal character could not be found in the portal class %s.", *GetName());
	ULocalPlayer* LP = portalController->GetLocalPlayer();
	CHECK_DESTROY_VOID(LogPortal, !LP, "UpdatePortalView: Portal player class couldn't be found in the portal %s.", *GetName());
	portalPlayer = LP;

	// Begin play ran.
	initialised = true;

	// Init last character location.
	//lastPawnLoc = portalCharacter->GetActorLocation();

	// If playing game and is game world setup delegate bindings.
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		// Bind the portals overlap events.
		portalBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnPortalBoxOverlapStart);
		portalBox->OnComponentEndOverlap.AddDynamic(this, &APortal::OnPortalBoxOverlapEnd);
		portalMesh->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnPortalMeshOverlapStart);
		portalMesh->OnComponentEndOverlap.AddDynamic(this, &APortal::OnPortalMeshOverlapEnd);
		GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this,
			&APortal::OnViewportResized);
	}

	//Set TargetPortal hide in portalCapture
	//portalCapture->HideActorComponents(pTargetPortal);
	portalCapture->HideComponent(pTargetPortal->portalMesh);
	portalCapture->HideComponent(pTargetPortal->portalBox);

	URTRPortalSubsystem* portalSubsystem = portalPlayer->GetSubsystem<URTRPortalSubsystem>();
	portalSubsystem->RegisterPortal(this);

	// After setup is done enable ticking functions.
	PrimaryActorTick.SetTickFunctionEnable(true);
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	// Delay setup for 2 seconds.
	PrimaryActorTick.SetTickFunctionEnable(false);
	FTimerHandle timer;
	FTimerDelegate timerDel;
	timerDel.BindUFunction(this, "Setup");
	GetWorldTimerManager().SetTimer(timer, timerDel, 1.0f, false, 1.0f);
}

void APortal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!initialised)
		return;
	URTRPortalSubsystem* portalSubsystem = portalPlayer->GetSubsystem<URTRPortalSubsystem>();
	portalSubsystem->ReleaseRenderTarget(this);

	FMinimalViewInfo playerViewInfo (portalController->PlayerCameraManager->ViewTarget.POV);
	if (!URTRPortalBPLibrary::IsInfront(playerViewInfo.Location, this, debugIsInfront))
		return;

	if (!URTRPortalBPLibrary::IsActorInCameraViewFrustum(this, playerViewInfo))
		return;
	
	portalMesh->cPortalMaterial->SetScalarParameterValue("ScaleOffset", 0.0f);

	FIntRect meshViewRect;
	playerViewInfo.bConstrainAspectRatio = false;//GetPixelPoint CAMERA: This has issues with aspect-ratio constrained cameras
	if (!portalMesh->CalculatePortalMeshRect(portalPlayer, playerViewInfo, meshViewRect, debugPortalMeshRect))
		return;

	portalSubsystem->AssignRenderTarget(this);
	
	FMinimalViewInfo captureViewInfo = portalCapture->UpdateSceneCapture(portalPlayer, playerViewInfo, this, meshViewRect, debugCameraTransform);

	portalCapture->CaptureScene();

	auto renderTarget = portalCapture->GetRenderTarget();
	portalMesh->cPortalMaterial->SetTextureParameterValue("RT_Portal", renderTarget);
	portalMesh->cPortalMaterial->SetVectorParameterValue("TargetSize",
		FLinearColor(meshViewRect.Min.X, meshViewRect.Min.Y, meshViewRect.Width(), meshViewRect.Height()));

	portalMesh->cPortalMaterial->SetScalarParameterValue("IsUVStable", portalCapture->bCaculateCustomProjectMatrix ? 0 : 1);

}

void APortal::OnPortalBoxOverlapStart(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit)
{
}

void APortal::OnPortalBoxOverlapEnd(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex)
{
}

void APortal::OnPortalMeshOverlapStart(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& portalHit)
{
}

void APortal::OnPortalMeshOverlapEnd(UPrimitiveComponent* portalMeshHit, AActor* overlappedActor, UPrimitiveComponent* overlappedComp, int32 otherBodyIndex)
{
}

void APortal::OnViewportResized(FViewport* viewport, UINT32 index)
{
}

